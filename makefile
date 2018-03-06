.PHONY: all clean

BIN_TARGET = DockerJudge/judger/safeJudger
BIN_SRC = JudgerSrc/safeJudger

all: $(BIN_TARGET)

$(BIN_TARGET): $(BIN_SRC)
	cp $(BIN_SRC) $(BIN_TARGET)

$(BIN_SRC):
	$(MAKE) -C JudgerSrc

clean:
	rm -f $(BIN_TARGET)
	$(MAKE) clean -C JudgerSrc