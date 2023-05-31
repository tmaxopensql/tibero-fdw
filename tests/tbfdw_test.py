import configparser
import argparse
import os
import subprocess
import pyodbc
import re
import random
import getpass

ODBC_DSN = "tbfdw_tibero"

class Arguments:
	def __init__(self):
		self.arg_parser = argparse.ArgumentParser(
			prog="python tbfdw_test.py",
			description="Regression Test for Tibero FDW",
			usage='%(prog)s [options] keywords...')

		self.arg_parser.add_argument(
			"keywords",
			metavar="keywords",
			nargs="*",
			help="Run test cases with names matching the given keywords."
		)

		self.arg_parser.add_argument(
			"-d",
			"--dry",
			dest="dry",
			action="store_true",
			help="Dry run. Show tests that would have run."
		)

		self.arg_parser.add_argument(
			"-q",
			"--quiet",
			dest="quiet",
			action="store_true",
			help="Only print summary results."
		)

		self.arg_parser.add_argument(
			"-w",
			"--without-tibero",
			dest="without_tibero",
			action="store_true",
			help="Run without Tibero connection. May cause failure if test cases need Tibero."
		)

		self.arg_parser.add_argument(
			"-t",
			"--trace_tbcli",
			dest="trace_tbcli",
			action="store_true",
			help="Leave log for Tibero client library."
		)

		self.arg_parser.add_argument(
			"-l",
			"--list",
			dest="show_list",
			action="store_true",
			help="Show group of keywords"
		)

		self.arg_parser.add_argument(
			"-i",
			"--init",
			dest="init_config",
			action="store_true",
			help="Initialize configuration with regard to connection infos"
		)

		self.arg_parser.add_argument(
			"-f",
			"--freeze",
			dest="freeze_on_exception",
			action="store_true",
			help="Freeze on exception. It will not rollback test environments on exception."
		)

	def parse(self):
		return self.arg_parser.parse_args()

class Path:
	TEST_CONFIG_FILE_NAME = "tbfdw_test.conf"
	TIBERO_INIT_SCHEMA_SQL_FILE_NAME = "tibero_init_schema.sql"
	TIBERO_INSERT_DATA_SQL_FILE_NAME = "tibero_insert_data.sql"
	TIBERO_ROLLBACK_SQL_FILE_NAME = "tibero_rollback.sql"
	ODBC_INI_FILE_NAME = "odbc.ini"
	TBODBC_LIBRARY_FILE_NAME = "libtbodbc.so"

	@staticmethod
	def get_project_home_dir():
		if os.environ.get("TBFDW_HOME") is None:
			raise Exception("TBFDW_HOME is not set. Please execute setenv.sh.")
		return os.environ.get("TBFDW_HOME")

	@staticmethod
	def get_tbcli_lib_dir():
		return os.path.join(Path.get_project_home_dir(), "lib")

	@staticmethod
	def get_test_home_dir():
		return os.path.join(Path.get_project_home_dir(), "tests")

	@staticmethod
	def get_test_cases_dir():
		return os.path.join(Path.get_test_home_dir(), "test_cases")

	@staticmethod
	def get_test_logs_dir():
		return os.path.join(Path.get_test_home_dir(), "logs")

	@staticmethod
	def get_test_config_file():
		return os.path.join(Path.get_test_home_dir(), Path.TEST_CONFIG_FILE_NAME)

	@staticmethod
	def get_tibero_init_schema_sql_file():
		return os.path.join(Path.get_test_home_dir(), Path.TIBERO_INIT_SCHEMA_SQL_FILE_NAME)

	@staticmethod
	def get_tibero_insert_data_sql_file():
		return os.path.join(Path.get_test_home_dir(), Path.TIBERO_INSERT_DATA_SQL_FILE_NAME)

	@staticmethod
	def get_tibero_rollback_sql_file():
		return os.path.join(Path.get_test_home_dir(), Path.TIBERO_ROLLBACK_SQL_FILE_NAME)

	@staticmethod
	def get_odbc_ini_file():
		return os.path.join(Path.get_test_home_dir(), Path.ODBC_INI_FILE_NAME)

	@staticmethod
	def get_tbodbc_lib_file():
		return os.path.join(Path.get_tbcli_lib_dir(), Path.TBODBC_LIBRARY_FILE_NAME)

	@staticmethod
	def file_exists(path):
		return os.path.isfile(path)


class Singleton(type):
	_instances = {}

	def __call__(cls, *args, **kwargs):
		if cls not in cls._instances:
			cls._instances[cls] = super(Singleton, cls).__call__(*args, **kwargs)
		return cls._instances[cls]

class TestDatabase(metaclass=Singleton):
	def __init__(self):
		self.dbname = "tbfdwtest" + str(random.getrandbits(32))
		self.created = False

	def __rep__(self):
		return f"<TestDatabase dbname:{self.dbname} created:{self.created}"

	def __str__(self):
		return f"TestDatabase with name {self.dbname}"

	def create(self, config):
		createdb_command = ["createdb"]

		createdb_command.append("-h")
		createdb_command.append(config["postgres"]["host"])
		self.host = config["postgres"]["host"]

		createdb_command.append("-p")
		createdb_command.append(config["postgres"]["port"])
		self.port = config["postgres"]["port"]

		createdb_command.append("-U")
		createdb_command.append(config["postgres"]["superuser"])
		self.user = config["postgres"]["superuser"]

		createdb_command.append("-O")
		createdb_command.append(config["postgres"]["superuser"])

		createdb_command.append("-w")

		createdb_command.append(self.dbname)

		os.environ["PGPASSWORD"] = config["postgres"]["password"]
		self.password = config["postgres"]["password"]

		subprocess.run(createdb_command)
		self.created = True

	def drop(self):
		dropdb_command = ["dropdb"]
		dropdb_command.append("-h")
		dropdb_command.append(self.host)
		dropdb_command.append("-p")
		dropdb_command.append(self.port)
		dropdb_command.append("-U")
		dropdb_command.append(self.user)
		dropdb_command.append(self.dbname)
		dropdb_command.append("-w")

		os.environ["PGPASSWORD"] = self.password
		subprocess.run(dropdb_command)
		self.created = False

class TiberoConnector(metaclass=Singleton):
	def __init__(self):
		self.connection = pyodbc.connect(f"DSN={ODBC_DSN}")

	def __del__(self):
		self.connection.close()

	def __rep__(self):
		return f"<TestDatabase dbname:{self.dbname} created:{self.created}"

	def __str__(self):
		return f"TestDatabase with name {self.dbname}"

	def check_tibero_health(self):
		cursor = self.connection.cursor()
		cursor.execute("select 'success' from dual;")
		row = cursor.fetchone()
		if row[0] != "success":
			raise Exception("Cannot connect to remote Tibero server")

	def execute_sql_script(self, script):
		cursor = self.connection.cursor()
		with open(script) as file:
			anon_block = file.read()
			cursor.execute(anon_block)

def upload_config_file():
	config_file_path = Path.get_test_config_file()
	if not Path.file_exists(config_file_path):
		raise Exception("Cannot find config file")

	config = configparser.ConfigParser()
	config.read(config_file_path)

	tb_user = config["tibero"]["user"]
	tb_passwd = getpass.getpass(f"Password for Tibero User ({tb_user}): ")
	config["tibero"]["password"] = tb_passwd

	pg_user = config["postgres"]["superuser"]
	pg_passwd = getpass.getpass(f"Password for Postgres Superuser ({pg_user}): ")
	config["postgres"]["password"] = pg_passwd

	return config

def show_list_of_test_cases():
	test_case_files = os.listdir(Path.get_test_cases_dir())
	test_case_names = {}

	for file in test_case_files:
		test_name = file.replace(".sql", "")
		prefix = test_name.partition("_")[0]

		if prefix in test_case_names:
			test_case_names[prefix].append(test_name)
		else:
			test_case_names[prefix] = [test_name]

	for prefix in sorted(test_case_names):
		print(prefix + ":")
		for test in test_case_names[prefix]:
			print("  -", test)

def prepare(config, args):
	check_pgtap_is_installed()
	check_odbc_is_installed()

	if not args.without_tibero and not args.dry:
		create_odbc_ini_file(config)
		set_tbcli_env_vars(args.trace_tbcli)
		check_tibero_health()
		initialize_tibero_test_schema()

	testdb = TestDatabase()
	testdb.create(config)

def check_pgtap_is_installed():
	p = subprocess.run(["pg_prove", "--version"], stdout=open(os.devnull, 'wb'))

	if p.returncode != 0:
		raise Exception("Please check if pgTAP is installed. Read README.md for instruction.")

def check_odbc_is_installed():
	p = subprocess.run(["isql", "--version"], stdout=open(os.devnull, 'wb'))

	if p.returncode != 0:
		raise Exception("Please check if unixodbc is installed. Read README.md for instruction.")

def create_odbc_ini_file(config):
	odbc_ini_file_path = Path.get_odbc_ini_file()

	with open(odbc_ini_file_path, "w") as odbc_ini:
		odbc_ini.write("[ODBC Data Source]\n")
		odbc_ini.write(f"{ODBC_DSN} = Tibero ODBC Driver\n")
		odbc_ini.write(f"[{ODBC_DSN}]\n")
		odbc_ini.write(f"Driver = {Path.get_tbodbc_lib_file()}\n")
		odbc_ini.write("Description = Tibero ODBC Driver\n")
		odbc_ini.write(f"Server = {config['tibero']['host']}\n")
		odbc_ini.write(f"Port = {config['tibero']['port']}\n")
		odbc_ini.write(f"Database = {config['tibero']['dbname']}\n")
		odbc_ini.write(f"User = {config['tibero']['user']}\n")
		odbc_ini.write(f"Password = {config['tibero']['password']}\n")

	os.environ["ODBCINI"] = odbc_ini_file_path

def set_tbcli_env_vars(trace_tbcli):
	# For TBCLI bug. Without this you get buffer overflow error.
	os.environ["TBCLI_WCHAR_TYPE"] = "UCS2"

	if trace_tbcli:
		os.environ["TBCLI_LOG_LVL"] = "TRACE"
		os.environ["TBCLI_LOG_DIR"] = Path.get_test_logs_dir()

def check_tibero_health():
	tb_connector = TiberoConnector()
	tb_connector.check_tibero_health()

def initialize_tibero_test_schema():
	print("Initializing test schema in the remote Tibero server.")
	print("This might take a while...")

	init_schema_script = Path.get_tibero_init_schema_sql_file()
	insert_data_script = Path.get_tibero_insert_data_sql_file()
	tb_connector = TiberoConnector()
	tb_connector.execute_sql_script(init_schema_script)
	tb_connector.execute_sql_script(insert_data_script)

	print("Done.")

def execute(config, args):
	test_cases_to_run = get_test_cases_to_run(args.keywords)
	pgtap_command = get_pgtap_command(config, args)

	for tests in test_cases_to_run:
		pgtap_command.append(os.path.join(Path.get_test_cases_dir(), tests))

	os.environ["PGPASSWORD"] = config["postgres"]["password"]
	subprocess.run(pgtap_command)

def get_test_cases_to_run(keywords):
	test_case_files = os.listdir(Path.get_test_cases_dir())
	test_cases_to_run = set()

	if len(keywords) == 0:
		test_cases_to_run = set(test_case_files)
	else:
		for keyword in keywords:
			for case in test_case_files:
				if re.search(re.escape(keyword), case):
					test_cases_to_run.add(case)

	return test_cases_to_run

def get_pgtap_command(config, args):
	pgtap_command = ["pg_prove", "--norc"]
	pgtap_command.append("--pset")
	pgtap_command.append("tuples_only=1")
	pgtap_command.append("--set")
	pgtap_command.append("PSQLRC=default")

	if args.dry:
		pgtap_command.append("--dry")
		print("Dry run. Here is the list of test cases that will be running:")

	if args.quiet:
		pgtap_command.append("--QUIET")
	else:
		pgtap_command.append("--verbose")

	pgtap_command.append("--host")
	pgtap_command.append(config["postgres"]["host"])

	pgtap_command.append("--port")
	pgtap_command.append(config["postgres"]["port"])

	pgtap_command.append("--dbname")
	pgtap_command.append(TestDatabase().dbname)

	pgtap_command.append("--username")
	pgtap_command.append(config["postgres"]["superuser"])

	pgtap_command.append("--set")
	pgtap_command.append("TIBERO_HOST=" + config["tibero"]["host"])

	pgtap_command.append("--set")
	pgtap_command.append("TIBERO_PORT=" + config["tibero"]["port"])

	pgtap_command.append("--set")
	pgtap_command.append("TIBERO_DB=" + config["tibero"]["dbname"])

	pgtap_command.append("--set")
	pgtap_command.append("TIBERO_USER=" + config["tibero"]["user"])

	pgtap_command.append("--set")
	pgtap_command.append("TIBERO_PASS=" + config["tibero"]["password"])

	return pgtap_command

def rollback():
	if Path.file_exists(Path.get_odbc_ini_file()):
		rollback_script = Path.get_tibero_rollback_sql_file()
		tb_connector = TiberoConnector()
		tb_connector.execute_sql_script(rollback_script)
		os.remove(Path.get_odbc_ini_file())

	testdb = TestDatabase()
	if testdb.created:
		testdb.drop()

	return

def main():
	args = Arguments().parse()
	try:
		if args.show_list:
			show_list_of_test_cases()
		else:
			config = upload_config_file()
			prepare(config, args)
			execute(config, args)
	except Exception as e:
		if not args.freeze_on_exception:
			rollback()
			raise e
	finally:
		rollback()

if __name__ == "__main__":
	main()
