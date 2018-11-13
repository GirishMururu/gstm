C=`pwd`

for dir in */;do
	cd $dir
	app="${dir::-1}"
	rm $app
	rm -rf timeagain_orig0_8_20
	cd $C
done
