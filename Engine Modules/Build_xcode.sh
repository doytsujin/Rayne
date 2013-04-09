BUILD_DIR="$PROJECT_DIR/../Build/$PRODUCT_NAME"
INCLUDE_DIR="$BUILT_PRODUCTS_DIR/usr/local/include"

mkdir -p "$BUILD_DIR"
mkdir -p "$BUILD_DIR/include"
cp "$BUILT_PRODUCTS_DIR/$EXECUTABLE_NAME" "$BUILD_DIR/$EXECUTABLE_NAME"


if [ -d "$INCLUDE_DIR" ]; then
    rsync -avz "$INCLUDE_DIR" "$BUILD_DIR"
fi