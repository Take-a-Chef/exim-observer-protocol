SHELL := /bin/sh
# Optional explicitly installed developer tools; no automatic downloads.
export PATH := $(CURDIR)/.tools/venv/bin:$(CURDIR)/.tools/bin:$(PATH)
CC ?= cc
FUZZ_CC ?= clang
CPPFLAGS += -Iinclude -Ic/include -Ibuild
WARNINGS := -Wall -Wextra -Wpedantic -Wformat=2 -Wshadow -Wconversion -Wsign-conversion -Wundef -Wcast-qual -Wwrite-strings -Wstrict-prototypes -Wmissing-prototypes -Werror=implicit-function-declaration
CFLAGS ?= -O2 -g
override CFLAGS += -std=c11 $(WARNINGS)
C_SOURCES := c/src/codec.c c/src/validation.c
C_HEADERS := $(wildcard include/*.h c/include/*.h c/src/*.inc)
C_TESTS := build/test_codec build/test_validation build/test_vectors
.PHONY: all generate check-generated validate-spec fmt fmt-check lint lint-c lint-docs test test-c test-vectors test-compat test-sanitize check fuzz-c fuzz-smoke clean tools-check
all: test
build:
	mkdir -p build
generate:
	python3 tools/generate.py
check-generated:
	./scripts/check-generated.sh
validate-spec:
	./scripts/validate-spec.sh
fmt:
	./scripts/check-format.sh --write
fmt-check:
	./scripts/check-format.sh
build/vectors.inc: $(wildcard testdata/*/*.json testdata/*/*.bin) tools/vectors.py | build
	python3 tools/vectors.py
build/test_%: c/tests/test_%.c $(C_SOURCES) $(C_HEADERS) build/vectors.inc
	$(CC) $(CPPFLAGS) $(CFLAGS) $(C_SOURCES) $< $(LDFLAGS) -o $@
test-c: $(C_TESTS)
	@set -e; for test in $(C_TESTS); do ./$$test; done
test: test-c
test-vectors:
	./scripts/check-testvectors.sh
test-compat: test-vectors
lint-c: build/vectors.inc
	clang-tidy $(C_SOURCES) c/tests/test_codec.c c/tests/test_validation.c c/tests/test_vectors.c fuzz/c/decode.c -- $(CPPFLAGS) -std=c11
	cppcheck --enable=warning,style,performance,portability --error-exitcode=1 --inline-suppr --std=c11 --suppress=missingIncludeSystem $(CPPFLAGS) $(C_SOURCES) c/tests/test_codec.c c/tests/test_validation.c c/tests/test_vectors.c fuzz/c/decode.c
lint-docs:
	yamllint -c .yamllint.yml spec .github .clang-format .clang-tidy .markdownlint.yaml .yamllint.yml
	./node_modules/.bin/markdownlint-cli2 '**/*.md' '!node_modules/**' '!.tools/**'
lint: lint-c lint-docs
check: fmt-check validate-spec check-generated lint test test-vectors test-compat
test-sanitize: CC = clang
test-sanitize: build/vectors.inc | build
	@set -e; for name in codec validation vectors; do \
	  $(CC) $(CPPFLAGS) -std=c11 $(WARNINGS) -O1 -g -fsanitize=address,undefined -fno-sanitize-recover=all -fno-omit-frame-pointer $(C_SOURCES) c/tests/test_$$name.c -o build/sanitize_$$name; \
	  ASAN_OPTIONS=detect_leaks=1 ./build/sanitize_$$name; \
	done
fuzz-c: | build
	$(FUZZ_CC) $(CPPFLAGS) -std=c11 $(WARNINGS) -O1 -g -fsanitize=fuzzer,address,undefined -fno-sanitize-recover=all $(C_SOURCES) fuzz/c/decode.c -o build/fuzz-decode
	mkdir -p build/corpus
	cp testdata/valid/*.bin testdata/invalid/*.bin build/corpus/
	./build/fuzz-decode build/corpus -max_total_time=10 -max_len=65561 -timeout=2
fuzz-smoke: fuzz-c
tools-check:
	./scripts/tools-check.sh
clean:
	rm -rf build
