# Historical pre-SIMD tree-only experiment

This historical experiment compares pre-SIMD main b15be548 against candidate b03c6410 on a different machine and compiler. It is not the stacked 31d238 → 4cb93dd9 remote experiment. Its control anomalies and memory measurements are preserved as separate evidence. The historical merge hold below describes that experiment, not the current stacked qualification.

## Historical decision

Independent review accepts the target asymmetric benefit and correctness. This is a qualified draft; merge is ON HOLD: control instability remains unresolved, so no confirmed no-regression claim is justified. No rerun fishing. Successful macOS and required CI checks on the exact merge-target head remain separately required.

## Candidate and qualification

Baseline is exact main b15be5484cf6397d754d5df3eca7d30fd3b84713. Candidate production SHA256 b03c641089c89b3d8d1d48a8d50e5e79edc7f71426c3886f7031e88bcc8d5408. This candidate contains no direct-evaluation SIMD or composition changes.

Two production edits: initialize product-tree size to 1 rather than degree-sized length/8; after the unchanged degree-sized reverse/div_series, erase the quotient prefix so evaluation receives its last size*8 coefficients. API, direct scalar route, runtime ISA routing, and coefficient representation unchanged. No new production header.

All omitted ancestors have right product {1}, so their product polynomial equals their left child. Old evaluate descends these ancestors by retaining the upper half each time, equivalent to retaining the final suffix once. Lower child transforms are unchanged. The old survivor-root frequency cache is no longer built, but evaluate uses child frequencies only. Generic PlainMint conversion remains unchanged.

Reference: official AC409516 https://judge.yosupo.jp/submission/409516, source lines7631–7646, inspected read-only. It uses a point-sized tree and quotient suffix window, but its division implementation differs; no source copied or executed.

## Tests

- Exact old baseline versus candidate and independent ordinary uint64 modular Horner:399 shapes each internal Mint and PlainMint. Includes empty, zero, duplicate/zero/maximal points, power-of-two boundaries, unequal dimensions, root product equality. Passed default, forced vectorized=false clone, ASan+UBSan (detect_leaks=0).
- Existing fps and fps_algorithms units passed.
- Retained new test/unit/multipoint_tree.cpp includes the shape grid, plus randomized and zero large asymmetric/point-heavy shapes, independently computed modular Horner and root product equality. Passed.
- Independent reviewer:236 additional adversarial cases (118 per Mint type), sparse/leading-zero, monomial, repeated +/-1/root-of-unity points, point-product-vanishing padded polynomials; default, forced scalar, ASan+UBSan all passed. Exact production source reviewed and approved.
- Apple Clang/macOS CI is not run locally and remains required before merge.

## Frozen measurement

Intel Xeon Platinum8573C, GCC14.2, CPU0, same-process exact baseline. GCC C++20 -O2 and -O2 -march=native.44 predetermined shapes,8 balanced alternating pairs, reversed shape traversal every other pair. Each variant >=0.15 actual process CPU seconds. Full output equality before each pair and full timed-result checksums. All704 paired observations retained; no reruns, exclusions, or tuning.

Before measurements, declared target n>=1024 and33<=m<=129. Each build required geometric mean of per-shape paired CPU medians>=1.10, at least75% target rows>=1.05, and no shape with paired median<1/1.05. Both builds pass this declared gate: target geometric means1.26943 portable and1.26211 native;100% target rows>=1.05. Target paired-median ranges are1.14060–1.39886 portable and1.11932–1.42218 native.

This does NOT mean every observation improved. Full summaries include absolute CPU/wall time and ratio-of-medians, which sometimes differs materially. In particular portable direct32×4096 has paired median0.98235 but ratio-of-medians0.85098, median675.5→793.8us. Its last individual pair was approximately1162→10771us (0.108x). Do not erase that outlier or call controls universally unchanged in measured performance. Independent review withholds automatic-merge acceptance because this control instability is unresolved; median-gate passage is not a blanket no-regression finding.

All adverse summary rows (n,m: paired CPU median / ratio of median CPU times):

Portable:
-4096,2048:1.03814 /0.97318
-32768,16384:0.99163 /1.01465
-1025,1025:0.99763 /0.94886
-32769,32769:0.97522 /1.00047
-33,1024:0.97999 /0.99864
-32,4096:0.98235 /0.85098
-4096,32:1.02921 /0.93875

Native:
-4096,2048:1.00563 /0.97542
-32768,4096:1.10366 /0.94535
-131072,131072:0.98545 /0.96436
-1025,1025:0.98539 /1.05100
-32769,32769:0.96467 /0.97848
-33,1024:0.96899 /0.99846
-129,32768:1.03053 /0.94991
-32,4096:1.02795 /0.97397
-4096,32:1.01504 /0.98793
-32768,16385:0.96650 /0.96096

## Memory, separately instrumented

Fresh subprocess per shape/variant, identical explicit baseline-then-candidate warmup, all new/delete and aligned allocation families counted. Report requested allocation bytes, not allocator overhead or RSS. Starting and ending live requested bytes matched in every final run; retained plan footprint was included in the starting state. A preliminary instrument assumption that counted live storage must return to zero failed; those pilot numbers are not used. The corrected continuous tracker exposes starting/peak/ending live bytes.

For131072 coefficients and129 points:
-Total peak live requested bytes6474840→4768564.
-Total allocated per operation5084500→3378224.
-Allocation count32834→129.
-Identical starting/ending live bytes1578500.
-Tree+frequency vector metadata1572864→3072bytes.
-Tree payload capacity138836→3376bytes;frequency payload6144→5120bytes.
-Quotient vector size131072→256 but capacity remains131072. Root division and its transform storage remain degree-sized.

Measured savings1706276bytes exactly equal structural metadata+payload savings1569792+135460+1024. The1024/33 and32768/33 shapes likewise agree exactly (13264 and426596bytes). Balanced131072/131072 has exactly identical allocation counts, peak bytes, and structural capacities. No process RSS or cold-plan claim is made.

## Complete per-shape summaries

CPU/wall values are median microseconds per call; paired ratios are median baseline/candidate pairs. ROM is ratio of independent CPU medians. Every frozen shape is included, not just favorable ones.

### portable

|n|m|Paired CPU|CPU ROM|Baseline CPU us|Candidate CPU us|Paired wall|Baseline wall us|Candidate wall us|
|---:|---:|---:|---:|---:|---:|---:|---:|---:|
|32|4096|0.98235|0.85098|675.5047|793.7953|0.98226|675.5645|793.8540|
|33|1024|0.97999|0.99864|244.9393|245.2733|0.97985|244.9539|245.3207|
|129|32768|1.02005|1.04394|13237.4679|12680.2852|1.01975|13237.8548|12681.1462|
|1024|33|1.21728|1.17850|75.3513|63.9383|1.21847|75.3445|63.9404|
|1024|63|1.30275|1.36195|68.7906|50.5088|1.30280|68.8057|50.5086|
|1024|65|1.28520|1.44987|77.5815|53.5094|1.28511|77.7193|54.8660|
|1024|128|1.29737|1.30679|78.7264|60.2439|1.29678|78.7380|60.2478|
|1024|129|1.23039|1.14549|82.4008|71.9353|1.21343|82.4006|71.9414|
|1024|512|1.06961|1.09295|146.6303|134.1601|1.06956|146.6312|134.1705|
|1024|1024|1.03857|1.09279|272.5935|249.4466|1.03843|272.6426|249.5348|
|1025|33|1.20191|1.22172|150.3407|123.0564|1.20078|150.5838|123.0709|
|1025|129|1.26780|1.15165|132.6418|115.1755|1.26783|132.6860|115.1753|
|1025|1025|0.99763|0.94886|349.5898|368.4322|0.99766|349.5907|368.4597|
|4096|32|1.02921|0.93875|907.1764|966.3644|1.02939|907.5398|987.0338|
|4096|33|1.27906|1.22026|301.2057|246.8371|1.26436|301.2063|246.8366|
|4096|63|1.32306|1.44071|249.9929|173.5201|1.32293|250.0099|173.5248|
|4096|65|1.36235|1.24328|244.1366|196.3654|1.36245|244.1369|196.3705|
|4096|129|1.39886|1.33192|248.4658|186.5475|1.43891|248.7898|186.9875|
|4096|512|1.15598|1.15652|307.8618|266.1975|1.15637|308.4906|266.2847|
|4096|2048|1.03814|0.97318|635.8371|653.3576|1.03813|635.8591|653.3552|
|4096|4096|1.01041|1.07012|1170.4847|1093.7838|1.01033|1170.5444|1095.8032|
|4097|33|1.25056|1.22070|704.7538|577.3379|1.25054|704.7749|577.3764|
|4097|129|1.30065|1.26424|477.4296|377.6421|1.30191|477.4308|377.7709|
|4097|1025|1.07543|1.06209|694.2605|653.6717|1.08834|696.5969|653.7209|
|4097|4097|1.02364|1.05541|1873.7026|1775.3359|1.02382|1873.6976|1781.0535|
|32768|33|1.15785|1.13631|2608.0884|2295.2239|1.15796|2608.4131|2295.4623|
|32768|63|1.23885|1.25496|1870.9756|1490.8624|1.23818|1870.9693|1490.9406|
|32768|65|1.22425|1.22126|1875.8168|1535.9644|1.17953|1875.8796|1576.3156|
|32768|129|1.37675|1.41501|1962.6149|1387.0014|1.39433|2006.4001|1386.9867|
|32768|4096|1.13882|1.25668|3040.2045|2419.2417|1.13854|3040.3800|2419.3216|
|32768|16384|0.99163|1.01465|6537.5322|6443.1518|0.99123|6537.9622|6450.5605|
|32768|16385|1.02411|1.01169|8055.8407|7962.7778|1.02412|8057.5870|7963.1790|
|32768|32768|1.09331|1.09329|13114.1253|11995.0655|1.09346|13115.8932|11995.0413|
|32769|33|1.14060|1.12262|5717.6517|5093.1171|1.13017|5723.3606|5107.1000|
|32769|129|1.34850|1.46438|4980.9905|3401.4393|1.34845|4981.7254|3401.5364|
|32769|1025|1.22902|1.15360|4399.5021|3813.7172|1.22891|4399.8157|3813.9469|
|32769|32769|0.97522|1.00047|17099.8533|17091.8234|0.97516|17100.9808|17094.1987|
|131072|33|1.21058|1.18868|11482.7391|9660.0874|1.21083|11484.0614|9660.4368|
|131072|63|1.23201|1.29301|8569.1173|6627.2605|1.21910|8595.0026|6729.0868|
|131072|65|1.30907|1.25640|8566.4251|6818.2424|1.29126|8566.6619|7058.4457|
|131072|129|1.27936|1.37319|8777.1203|6391.7880|1.27955|8777.3965|6393.4866|
|131072|16384|1.14300|1.25064|14590.5343|11666.4757|1.14308|14593.3213|11667.4844|
|131072|65536|1.02183|1.02391|31411.7543|30678.3505|1.02186|31413.8150|30681.9659|
|131072|131072|1.03917|1.12540|70777.2408|62890.4995|1.03856|72004.1177|62893.4932|

### native

|n|m|Paired CPU|CPU ROM|Baseline CPU us|Candidate CPU us|Paired wall|Baseline wall us|Candidate wall us|
|---:|---:|---:|---:|---:|---:|---:|---:|---:|
|32|4096|1.02795|0.97397|668.0517|685.9092|1.01045|668.0760|701.9705|
|33|1024|0.96899|0.99846|245.1853|245.5640|0.96899|245.1840|245.5641|
|129|32768|1.03053|0.94991|12828.5270|13504.9863|1.01156|12832.8295|13511.6592|
|1024|33|1.18128|1.15328|69.9345|60.6397|1.18122|71.3058|60.6722|
|1024|63|1.42218|1.70898|85.4134|49.9793|1.45887|87.1184|49.9847|
|1024|65|1.28460|1.41940|83.0868|58.5366|1.28477|83.0890|58.5411|
|1024|128|1.25318|1.24465|71.3467|57.3229|1.25324|71.3517|57.3245|
|1024|129|1.19868|1.24633|87.8217|70.4640|1.19890|87.8531|70.4639|
|1024|512|1.04962|1.06988|147.0996|137.4911|1.05013|147.1280|137.4993|
|1024|1024|1.04807|1.03711|256.5811|247.3999|1.06652|261.5596|252.9280|
|1025|33|1.21258|1.21026|145.6381|120.3365|1.21240|145.6422|120.3660|
|1025|129|1.22493|1.19795|132.5561|110.6522|1.22395|132.5689|110.6518|
|1025|1025|0.98539|1.05100|357.9559|340.5863|1.00124|357.9645|340.6292|
|4096|32|1.01504|0.98793|902.8652|913.8952|1.01538|903.3467|913.8989|
|4096|33|1.22754|1.24913|281.8950|225.6739|1.22734|281.8941|225.7529|
|4096|63|1.38929|1.58471|255.3919|161.1600|1.41734|255.3913|161.1644|
|4096|65|1.33326|1.42150|233.2559|164.0919|1.33345|233.2628|164.1291|
|4096|129|1.28932|1.19527|264.1710|221.0142|1.28867|264.2071|221.0193|
|4096|512|1.25030|1.11742|332.5005|297.5617|1.27498|332.5003|297.5824|
|4096|2048|1.00563|0.97542|656.6914|673.2411|0.98296|656.7267|673.2380|
|4096|4096|1.09281|1.14239|1467.8225|1284.8659|1.09283|1468.0380|1317.1472|
|4097|33|1.28369|1.26528|586.9998|463.9280|1.28405|587.1941|466.4450|
|4097|129|1.27098|1.27632|456.7292|357.8490|1.27098|456.7264|357.8479|
|4097|1025|1.07886|1.09704|660.0841|601.6982|1.07889|660.0839|601.7029|
|4097|4097|1.07817|1.01473|1609.3589|1585.9950|1.07832|1609.6954|1586.0465|
|32768|33|1.20341|1.57863|3598.3675|2279.4195|1.20342|3598.6730|2279.4096|
|32768|63|1.27252|1.29687|1974.3607|1522.3993|1.24537|1974.4946|1558.3811|
|32768|65|1.33765|1.30823|2021.2458|1545.0277|1.33608|2021.3202|1545.0680|
|32768|129|1.28388|1.15445|1864.5510|1615.1039|1.28357|1864.7431|1615.4090|
|32768|4096|1.10366|0.94535|2940.8572|3110.8522|1.12524|2940.8459|3111.1056|
|32768|16384|1.10880|1.14590|7627.6294|6656.4574|1.13522|7880.8734|6658.4544|
|32768|16385|0.96650|0.96096|7694.1318|8006.6989|0.96650|7915.0040|8007.0001|
|32768|32768|1.05152|1.02839|12666.1780|12316.5046|1.05174|12668.0711|12318.3060|
|32769|33|1.11932|1.11764|5172.7171|4628.2362|1.11936|5255.5634|4628.6098|
|32769|129|1.32749|1.29721|4141.4247|3192.5721|1.32726|4143.4523|3192.8391|
|32769|1025|1.28188|1.28254|4522.0782|3525.8805|1.28147|4578.7967|3527.3024|
|32769|32769|0.96467|0.97848|16694.5189|17061.7228|0.96467|16696.2776|17061.6839|
|131072|33|1.20087|1.25419|12812.8797|10216.0471|1.16594|12815.6807|10548.4430|
|131072|63|1.27391|1.26192|8257.2083|6543.3807|1.27390|8258.3975|6543.5370|
|131072|65|1.24045|1.20411|8093.1383|6721.2719|1.23934|8093.1036|6727.5566|
|131072|129|1.23805|1.22904|8096.3065|6587.4916|1.22462|8097.1863|6655.4211|
|131072|16384|1.11280|1.09148|12926.8480|11843.3979|1.11215|12927.2126|11872.5650|
|131072|65536|1.05503|1.08507|33552.3102|30921.6944|1.05533|33556.7404|30921.5386|
|131072|131072|0.98545|0.96436|62013.3067|64305.3300|0.98553|62021.3080|64310.6968|

