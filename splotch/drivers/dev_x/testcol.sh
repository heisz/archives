echo
echo "Default"
./sp_x $* ../drv_test.sdvi
echo
echo "StaticColor"
./sp_x $* -vt StaticColor ../drv_test.sdvi
echo
echo "PseudoColor"
./sp_x $* -vt PseudoColor ../drv_test.sdvi
echo
echo "StaticGray"
./sp_x $* -vt StaticGray ../drv_test.sdvi
echo
echo "GrayScale"
./sp_x $* -vt GrayScale ../drv_test.sdvi
echo
echo "TrueColor"
./sp_x $* -vt TrueColor ../drv_test.sdvi
echo
echo "DirectColor"
./sp_x $* -vt DirectColor ../drv_test.sdvi
