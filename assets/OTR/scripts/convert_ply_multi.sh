for file in ../ply/*.ply; do
	nohup ./ptf "$( basename $file )"  > /dev/null 2>&1 &
done;