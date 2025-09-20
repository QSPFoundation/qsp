package com.libqsp.jni;

public abstract class QSPLib {
    public enum Error {
        DIVBYZERO, /* = 10 */
        TYPEMISMATCH,
        STACKOVERFLOW,
        TOOMANYITEMS,
        CANTLOADFILE,
        GAMENOTLOADED,
        COLONNOTFOUND,
        CANTINCFILE,
        CANTADDACTION,
        EQNOTFOUND,
        LOCNOTFOUND,
        ENDNOTFOUND,
        LABELNOTFOUND,
        INCORRECTNAME,
        QUOTNOTFOUND,
        BRACKNOTFOUND,
        BRACKSNOTFOUND,
        SYNTAX,
        UNKNOWNACTION,
        ARGSCOUNT,
        CANTADDOBJECT,
        CANTADDMENUITEM,
        TOOMANYVARS,
        INCORRECTREGEXP,
        CODENOTFOUND,
        LOOPWHILENOTFOUND
    }

    public final class Window {
        public static final int MAIN  = 1 << 0;
        public static final int VARS  = 1 << 1;
        public static final int ACTS  = 1 << 2;
        public static final int OBJS  = 1 << 3;
        public static final int INPUT = 1 << 4;
        public static final int VIEW  = 1 << 5;

        private int value;

        public Window() {
            this.value = 0;
        }

        public Window(int value) {
            this.value = value;
        }

        public int getValue() {
            return value;
        }

        public boolean isSet(int flags) {
            return (value & flags) == flags;
        }

        public Window set(int flags) {
            value |= flags;
            return this;
        }

        public Window clear(int flags) {
            value &= ~flags;
            return this;
        }
    }

    public class ListItem {
        public String name;
        public String image;
    }

    public class ObjectItem {
        public String name;
        public String title;
        public String image;
    }

    public class ExecutionState {
        public String loc;
        public int actIndex;
        public int lineNum;
    }

    public class ErrorInfo {
        public int errorNum; /* Error.ordinal() */
        public String errorDesc;
        public String locName; /* location name */
        public int actIndex; /* index of the base action */
        public int topLineNum; /* top-level line within the game code */
        public int intLineNum; /* line number of the actual code */
        public String intLine; /* line of the actual code */
    }

    static {
        System.loadLibrary("qsp");
        // System.load("libqsp.so");
    }

    // Main API
    public native void init();
    public native void terminate();

    public native void enableDebugMode(boolean isDebug);
    public native ExecutionState getCurrentState();
    public native String getVersion();
    public native String getCompiledDateTime();
    public native String getMainDesc();
    public native String getVarsDesc();
    public native void setInputStrText(String value);
    public native ListItem[] getActions();
    public native boolean setSelActIndex(int index, boolean toRefreshUI);
    public native boolean execSelAction(boolean toRefreshUI);
    public native int getSelActIndex();
    public native ObjectItem[] getObjects();
    public native boolean setSelObjIndex(int index, boolean toRefreshUI);
    public native int getSelObjIndex();
    public native int getWindowsChangedState(); /* Window bit flags */
    public native void showWindow(int type /* Window bit flags */, boolean toShow);
    public native int getVarValuesCount(String name);
    public native int getVarIndexByString(String name, String str);
    /*
    public native QSPVariant getVarValue(String name, int index);
    public native String convertValueToString(QSPVariant value);
    */
    public native long getNumVarValue(String name, int index);
    public native String getStrVarValue(String name, int index);
    public native boolean execString(String code, boolean toRefreshUI);
    public native String calculateStrExpr(String expression, boolean toRefreshUI);
    public native long calculateNumExpr(String expression, boolean toRefreshUI);
    public native boolean execLocationCode(String name, boolean toRefreshUI);
    public native boolean execCounter(boolean toRefreshUI);
    public native boolean execUserInput(boolean toRefreshUI);
    public native ErrorInfo getLastErrorData();
    public native String getErrorDesc(int errorNum);
    public native boolean loadGameWorldFromData(byte[] data, boolean isNewGame);
    public native byte[] saveGameAsData(boolean toRefreshUI);
    public native boolean openSavedGameFromData(byte[] data, boolean toRefreshUI);
    public native boolean restartGame(boolean toRefreshUI);

    // Callbacks
    public void onDebug(String str) {}
    public boolean onIsPlayingFile(String file) { return false; }
    public void onPlayFile(String file, int volume) {}
    public void onCloseFile(String file) {}
    public void onShowImage(String file) {}
    public void onShowWindow(int type /* Window bit flags */, boolean toShow) {}
    public int onShowMenu(ListItem[] items) { return -1; }
    public void onShowMessage(String text) {}
    public void onRefreshInt(boolean isForced, boolean isNewDesc) {}
    public void onSetTimer(int msecs) {}
    public void onSetInputStrText(String text) {}
    public void onSystem(String cmd) {}
    public void onOpenGame(String file, boolean isNewGame) {}
    public void onInitGame(boolean isNewGame) {}
    public void onOpenGameStatus(String file) {}
    public void onSaveGameStatus(String file) {}
    public void onSleep(int msecs) {}
    public int onGetMsCount() { return 0; }
    public String onInputBox(String text) { return ""; }
    public String onVersion(String param) { return ""; }
}
