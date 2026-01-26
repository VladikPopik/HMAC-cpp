echo "Create Virtual env and start test for rps of me microservice"

python3 -m venv test_venv

source ./test_venv/bin/activate

python3 -m pip install -r requirements.txt

source ./test_venv/bin/activate

python3 test_rps.py 10 100