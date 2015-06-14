# ldif_csv

Fast serialization of LDIF into CSV in C (https://github.com/szechuen/ldif_csv)


### Installation

```bash
git clone https://github.com/szechuen/ldif_csv.git
cd ldif_csv
make
```

Pre-requisite: **gcc** or **clang** (change Makefile if necessary)


### Usage

```bash
./ldif_csv <input_file.ldif> <output_file.csv>
```

where `<input_file.ldif>` is of type LDIF **(WITHOUT LINE WRAP!)** and `<output_file.csv>` is of type CSV

##### Sample input LDIF retrieval

```bash
ldapsearch -x -H ldap://<host>:<port> -b "<search_base>" -s sub -LLL -o ldif-wrap=no "<filter>" > <input_file.ldif>
```

Note: Use `-LLL` to disable all comments and `-o ldif-wrap=no` to disable line wrapping


### Performance

**~500k records, ~100 keys:** ~21s on MacBook Pro (2.2 GHz Intel Core i7) [Sze Chuen Tan]

Submit functionality bugs, security vulnerabilities and performance reports under Github Issues (https://github.com/szechuen/ldif_csv/issues). 