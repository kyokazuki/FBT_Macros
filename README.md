# FBT_Macros
## Acquiring
### acquire_ov_th.sh  
Acquire data for multiple voltage and threshold settings. Run numbers are automatically detected.  Edit the variables in the script and run from the `build/` directory:
```
sh acquire_ov_th.sh
```

## Analysing
### plot_ov_effcy.C  
Plots efficiency to voltage with multiple data files in ROOT. 
```
root plot_ov_effcy.C
```

### plot_th_xi_cnt.C
Plots a 3D threshold-xi distribution of events in ROOT. 
```
root plot_th_xi_cnt.C
```

### plot_time_diff.C
Plots the time distribution of events relative to a trigger in ROOT.
```
root
[0] .L plot_time_diff.C+
[1] time_diff("data_file.root")
```

### print_ov_mult_cnt.C
Creates multiplicity distribution graphs for multiple volatges as a PDF.
```
root print_ov_mult_cnt.C
```

### print_ov_th_rate.C
Creates threshold-rate plots for multiple voltages as a PDF.
```
root print_ov_th_rate.C
```

### print_th_tot_cnt.C
Creates tot distributions for multiple thresholds as a PDF.
```
root print_th_tot_cnt.C
```

### tree_group_events.C
Groups multiple entries within a timeframe relative to a trigger into one entry with vector values.
```
root
[0] .L tree_group_events.C+
[1] treeGroupEvents("data_file")
```

### tree_group_events_2.C
Group multiple entries within a timeframe relative to a trigger into one entry with vector values. Branches are divided into X, Y, U to seperate elements by surfaces. 
```
root
[0] .L tree_group_events_2.C+
[1] treeGroupEvents("data_files")
```

### tree_friend.C
Friend entries from two ROOT data files. Third argument offsets the tree with fewer entries.
```
root
[0] .L tree_friend.C+
[1] treeFriend("data_file_1", "data_file_2", 0)
```

### walk.C
Plot time-tot graph relative to a trigger and correct slew by fitting.
```
root
[0]  .L walk.C+
[1] multiplicity("data_file")
```

