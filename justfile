alias bk := build_langsud
alias bm := build_manager

build_langsud:
    cross build --target aarch64-linux-android --release

build_manager: build_langsud
    cp target/aarch64-linux-android/release/langsud manager/app/src/main/jniLibs/arm64-v8a/liblangsud.so
    cd manager && ./gradlew aDebug

clippy:
    cargo fmt
    cross clippy --target aarch64-linux-android --release
