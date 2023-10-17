# TC-Match

## Introduction


## Compile

Our framework requires C++ 20 and CMake 3.16 (or later). One can compile the code by executing the following commands.

```bash
cd ~/Downloads/TC-Match

mkdir build

cd build

cmake ..

make
```

## Execute

After a successful compilation, the binary file (`tcm`) is created under the `TC-Match/build/` directory . One can execute tcm using the following command.

```bash
./tcm -s <stream-data-path> -q <time-constraint-query-path>
```

### Optional parameters

| Parameters. | Explanation                                               | Default Value |
| ----------- | --------------------------------------------------------- | ------------- |
| -r          | result file path                                          | ./            |
| --em        | the mode of executing search [count/enum]                 | count         |
| --ism       | is consider using static merge [y/n]                      | y             |
| --idm       | is consider duplication [y/n]                             | y             |
| --pct       | regard how much percentage stream edges as the data graph | 0.6           |
| --itl       | max index time                                            | 36000         |
| --otl       | max online time                                           | 3600          |
| --rm        | the mode of showing result [time/qid]                     | time          |

After executing this command, the `match.result` file will be created under the `result file path` directory.

## Input File Format

Both the stream graph and query graph are edge-based. 

Each stream edge is represented by: `(v_source_id,v_target_id,edge_label,v_source_label,v_target_label,timestamp)`. 

Each query edge is represented by:  `(qid,u_source_id,u_target_id,edge_label,u_source_label,u_target_label)`.

### Query Graph

Each line in the query graph file represent an edge or a time-constraint order.

1. An edge is represented by `e <qid> <u_source_id> <u_target_id> <edge_label> <u_source_label> <u_target_label>`.

2. A time-constrain is represented by `b <qid1> <qid2> ... <qidn>`,which means <qid1> must arrive before <qid2>, likewise, <qid3> must arrive before <qid3>,...

For example,
```
e 0 1 2 0 80 80
e 1 1 3 1 443 21 
e 2 2 3 0 8080 22
b 0 1 2
```

### Stream Graph

Each line in the stream graph file represent a time-edge.

An edge is represented by `<v_source_id> <v_target_id> <edge_label> <v_source_label> <v_target_label> <timestamp>`.

For example:
```
10 20 0 8080 80 1
10 30 1 443 21 2
20 30 0 8080 22 3
```

### Test
```bash
cd ~/Downloads/TC-Match/test

bash tcm.sh
```



## Datasets and Querysets

The stream graph datasets and their corresponding querysets used in our paper can be downloaded [here](https://drive.google.com/drive/folders/1h7SrPfxYlk8z1A8CnFGk5WqpZEHsA29R?usp=sharing).
