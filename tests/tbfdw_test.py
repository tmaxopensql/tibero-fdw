import configparser
import argparse
import os
import subprocess
import pyodbc
import re

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

	def parse(self):
		return self.arg_parser.parse_args()

class Path:
	TEST_CONFIG_FILE_NAME = "tbfdw_test.conf"
	TIBERO_INIT_SQL_FILE_NAME = "tibero_init.sql"
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
	def get_tibero_init_sql_file():
		return os.path.join(Path.get_test_home_dir(), Path.TIBERO_INIT_SQL_FILE_NAME)

	@staticmethod
	def get_odbc_ini_file():
		return os.path.join(Path.get_test_home_dir(), Path.ODBC_INI_FILE_NAME)

	@staticmethod
	def get_tbodbc_lib_file():
		return os.path.join(Path.get_tbcli_lib_dir(), Path.TBODBC_LIBRARY_FILE_NAME)

	@staticmethod
	def file_exists(path):
		return os.path.isfile(path)

def create_config_file_if_not_exist():
	config_file = Path.get_test_config_file()

	if Path.file_exists(config_file):
		return

	tb_conn_str = input("Type in connection string for Tibero [host:port:dbname:user:password]: ")
	write_conn_str_to_config_file("tibero", tb_conn_str)

	pg_conn_str = input("Type in connection string for PostgreSQL [host:port:dbname:user:password]: ")
	write_conn_str_to_config_file("postgres", pg_conn_str)

	print()
	print("Config file is created at " , config_file, ".")
	print("Please edit the config file directly if you want to change the connection information.")

def write_conn_str_to_config_file(section, conn_str):
	config_file_path = Path.get_test_config_file()
	conn_str_format = "host:port:dbname:user:password"
	normal_conn_str_size = len(conn_str_format.split(":"))

	conn_str_split = conn_str.split(":")

	if len(conn_str_split) != normal_conn_str_size:
		raise Exception("Wrong connection string format")

	config_file = open(config_file_path, "a")
	config_file.write("[" + section + "]\n")
	config_file.write("host=" + conn_str_split[0] + "\n")
	config_file.write("port=" + conn_str_split[1] + "\n")
	config_file.write("dbname=" + conn_str_split[2] + "\n")
	config_file.write("user=" + conn_str_split[3] + "\n")
	config_file.write("password=" + conn_str_split[4] + "\n")
	config_file.write("\n")
	config_file.close()

def upload_config_file():
	config_file_path = Path.get_test_config_file()
	config = configparser.ConfigParser()
	config.read(config_file_path)
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
	
	if not args.without_tibero and not args.dry:
		create_odbc_ini_file_if_not_exist(config)
		set_tbcli_env_vars(args.trace_tbcli)
		check_tibero_health()
		initialize_tibero_test_schema()

def check_pgtap_is_installed():
	p = subprocess.run(["pg_prove", "--version"], stdout=open(os.devnull, 'wb'))

	if p.returncode != 0:
		raise Exception("Please check if pgTAP is installed. Read README.md for instruction.")

def create_odbc_ini_file_if_not_exist(config):
	odbc_ini_file_path = Path.get_odbc_ini_file()

	if not Path.file_exists(odbc_ini_file_path):
		with open(odbc_ini_file_path, "w") as odbc_ini:
			odbc_ini.write("[ODBC Data Source]" + "\n")
			odbc_ini.write("tbfdw_tibero = Tibero ODBC Driver" + "\n")
			odbc_ini.write("[tbfdw_tibero]" + "\n")
			odbc_ini.write("Driver = " + Path.get_tbodbc_lib_file() + "\n")
			odbc_ini.write("Description = Tibero ODBC Driver" + "\n")
			odbc_ini.write("Server = " + config["tibero"]["host"] + "\n")
			odbc_ini.write("Port = " + config["tibero"]["port"] + "\n")
			odbc_ini.write("Database = " + config["tibero"]["dbname"] + "\n")
			odbc_ini.write("User = " + config["tibero"]["user"] + "\n")
			odbc_ini.write("Password = " + config["tibero"]["password"] + "\n")

	os.environ["ODBCINI"] = odbc_ini_file_path

def set_tbcli_env_vars(trace_tbcli):
	# For TBCLI bug. Without this you get buffer overflow error.
	os.environ["TBCLI_WCHAR_TYPE"] = "UCS2"

	if trace_tbcli:
		os.environ["TBCLI_LOG_LVL"] = "TRACE"
		os.environ["TBCLI_LOG_DIR"] = Path.get_test_logs_dir()

def check_tibero_health():
	with pyodbc.connect("DSN=tbfdw_tibero") as conn:
		cursor = conn.cursor()
		cursor.execute("select 'success' from dual;")
		row = cursor.fetchone()
		if row[0] != "success":
			raise Exception("Cannot connect to remote Tibero server")

def initialize_tibero_test_schema():
	print("Initializing test schema in the remote Tibero server.")
	print("This might take a while...")

	conn = pyodbc.connect("DSN=tbfdw_tibero")
	cursor = conn.cursor()
	sql_script = Path.get_tibero_init_sql_file()

	with open(sql_script, "r") as sql_file:
		stmt = ""
		for line in sql_file:
			if line.startswith("--"):
				continue

			stmt = stmt + line
			if ";" in line:
				try:
					cursor.execute(stmt)
				except Exception:
					print("Exception occurred, but we will ignore it: " + stmt)
				stmt = ""
	
	print("Done.")

def execute(config, args):
	test_cases_to_run = get_test_cases_to_run(args.keywords)
	pgtap_command = get_pgtap_command(config, args)
	
	for tests in test_cases_to_run:
		pgtap_command.append(os.path.join(Path.get_test_cases_dir(), tests))

	os.environ["PGPASSWORD"] = config["postgres"]["password"]
	subprocess.run(pgtap_command)
	del os.environ["PGPASSWORD"]

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
	pgtap_command.append(config["postgres"]["dbname"])

	pgtap_command.append("--username")
	pgtap_command.append(config["postgres"]["user"])

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

def main():
	create_config_file_if_not_exist()

	config = upload_config_file()
	args = Arguments().parse()

	if args.show_list:
		show_list_of_test_cases()
	else:
		prepare(config, args)
		execute(config, args)

if __name__ == "__main__":
	main()
