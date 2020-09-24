include config.mk

all: build

venv: venv/bin/activate

venv/bin/activate: requirements.txt
	@test -d venv || virtualenv -p /usr/bin/python3 venv
	@./venv/bin/pip install -Ur requirements.txt
	@touch ./venv/bin/activate

prepare: venv
	@./venv/bin/pip install -r requirements.txt

build:
	@:

run: venv
	@./venv/bin/python3 ${PROJECT}.py ${DATABASE} ${PORT}

clean:
	@rm -rf venv
