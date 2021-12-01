// skip comment
// db/io: read_write_db starts from 100:
100 "Successfully read design %s.\n"
	{detail message}

101 "Successfully write design %s.\n"
	{detail message}

102 "Failed to open file %s.\n"
	{detail message}

103 "Failed in checksum: %d (ref_value %d).\n"
	{detail message}

104 "Succeeded in checksum.\n"
	{detail message}

105 "Top cell %s with ID#%ld.\n        Tech lib with ID#%ld (type %d).\n        Timing lib with ID#%ld (type %d).\n"
	{detail message}

106 "Failed to init %s.\n"
	{detail message}

107 "Failed to create directory %s.\n"
	{detail message}

108 "Failed to write %s for %s.\n"
	{detail message}

109 "Rename top cell %s to %s.\n"
	{detail message}

// db/core: cell
1 "Create %s %s failed.\n"
	{}

2 "Create %s failed.\n"
	{}

3 "Create %s %s failed due to name conflicts.\n"
	{}

4 "Cannot find Tech LEF.\n"
	{}

5 "Class type %s not supported.\n"
	{}

6 "Sub class type %s not supported.\n"
	{}

7 "Index %d is more than %d, the number of component mask shift layers.\n"
	{}

// db/core: db_tcl_command
8 "read_lef command should be run first.\n"
	{}

9 "Invalid DB file name.\n"
	{}

10 "Failed to get top cell.\n"
	{}

11 "Failed to init top cell.\n"
	{}

12 "Create floorplan failed when initializing top cell.\n"
	{}

// db/core: fplan
13 "Site name is null.\n"
	{}

14 "Cannot find Tech LEF when reading Track.\n"
	{}

15 "Cannot find layer %s in LEF.\n"
	{}

16 "Cannot find Tech LEF when print Track.\n"
	{}

17 "Cannot get polygon tale when getting die area.\n"
	{}

18 "Blockage's component is nullptr.\n"
	{}

19 "Cannot find component %s in blockage.\n"
	{}

20 "Cannot find instance with object id %d.\n"
	{}

// db/core: inst
21 "Cannot find cell %s for instance %s.\n"
	{}

22 "The cell of instance %s is null.\n"
	{}

23 "The eeq master cell of instance %s is null.\n"
	{}

24 "Cannot find top cell when set term name %s.\n"
	{}

25 "Cannot find top cell.\n"
	{}

26 "Antenna Area find layer %s failed.\n"
	{}

27 "Antenna Area find layer index %d larger than 255.\n"
	{}

// db/util: geometrys
28 "Layer Geometry find layer %s failed.\n"
	{}

29 "Layer Geometry find layer index %d larger than 255.\n"
	{}

30 "GeometryVia find via master %s failed.\n"
	{}

31 "There are not symbol pages.\n"
	{}

// db/util: property_manager
32 "Property definition %s has already been created.\n"
	{}

33 "Property %s has not been created yet.\n"
	{}

34 "Property %s has %s type.\n"
	{}

35 "Property %s get nullptr input.\n"
	{}

36 "Property %s data type error.\n"
	{}

37 "Cannot find property %s.\n"
	{}
