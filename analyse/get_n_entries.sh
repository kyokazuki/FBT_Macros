run_num=$1
run_num_formatted=$(printf "%04d" ${run_num})
data=$(ls ${run_num_formatted}*_ext.root)
echo $data
root -l ${data} <<EOF
	TTree *tree = (TTree*) gDirectory->Get("data")
	cout << "Total entries: " << endl
	tree->GetEntries()
	cout << "Gate-start entries: " << endl
	tree->Draw("xi", "xi==0 && energy==5", "goff")
	.q
EOF	

