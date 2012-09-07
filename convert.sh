#!/bin/sh
objconvert -yz -gl -n topgum -d -s Cube_001-mesh -o topgum.inc teeth.dae
objconvert -yz -gl -n bottomgum -d -s Cube-mesh -o bottomgum.inc teeth.dae

objconvert -yz -gl -n tl1 -d -s Cube_003-mesh -o tl1.inc teeth.dae
objconvert -yz -gl -n tl2 -d -s Cube_004-mesh -o tl2.inc teeth.dae
objconvert -yz -gl -n tl3 -d -s Cube_006-mesh -o tl3.inc teeth.dae
objconvert -yz -gl -n tl4 -d -s Cube_008-mesh -o tl4.inc teeth.dae
objconvert -yz -gl -n tl5 -d -s Cube_009-mesh -o tl5.inc teeth.dae
objconvert -yz -gl -n tl6 -d -s Cube_010-mesh -o tl6.inc teeth.dae
objconvert -yz -gl -n tl7 -d -s Cube_011-mesh -o tl7.inc teeth.dae

objconvert -yz -gl -n tr1 -d -s Cube_002-mesh -o tr1.inc teeth.dae
objconvert -yz -gl -n tr2 -d -s Cube_005-mesh -o tr2.inc teeth.dae
objconvert -yz -gl -n tr3 -d -s Cube_007-mesh -o tr3.inc teeth.dae
objconvert -yz -gl -n tr4 -d -s Cube_012-mesh -o tr4.inc teeth.dae
objconvert -yz -gl -n tr5 -d -s Cube_013-mesh -o tr5.inc teeth.dae
objconvert -yz -gl -n tr6 -d -s Cube_014-mesh -o tr6.inc teeth.dae
objconvert -yz -gl -n tr7 -d -s Cube_015-mesh -o tr7.inc teeth.dae

objconvert -yz -gl -n bl1 -d -s Cube_016-mesh -o bl1.inc teeth.dae
objconvert -yz -gl -n bl2 -d -s Cube_017-mesh -o bl2.inc teeth.dae
objconvert -yz -gl -n bl3 -d -s Cube_018-mesh -o bl3.inc teeth.dae
objconvert -yz -gl -n bl4 -d -s Cube_019-mesh -o bl4.inc teeth.dae
objconvert -yz -gl -n bl5 -d -s Cube_024-mesh -o bl5.inc teeth.dae
objconvert -yz -gl -n bl6 -d -s Cube_027-mesh -o bl6.inc teeth.dae
objconvert -yz -gl -n bl7 -d -s Cube_028-mesh -o bl7.inc teeth.dae
objconvert -yz -gl -n bl8 -d -s Cube_031-mesh -o bl8.inc teeth.dae

objconvert -yz -gl -n br1 -d -s Cube_020-mesh -o br1.inc teeth.dae
objconvert -yz -gl -n br2 -d -s Cube_021-mesh -o br2.inc teeth.dae
objconvert -yz -gl -n br3 -d -s Cube_022-mesh -o br3.inc teeth.dae
objconvert -yz -gl -n br4 -d -s Cube_023-mesh -o br4.inc teeth.dae
objconvert -yz -gl -n br5 -d -s Cube_025-mesh -o br5.inc teeth.dae
objconvert -yz -gl -n br6 -d -s Cube_026-mesh -o br6.inc teeth.dae
objconvert -yz -gl -n br7 -d -s Cube_029-mesh -o br7.inc teeth.dae
objconvert -yz -gl -n br8 -d -s Cube_030-mesh -o br8.inc teeth.dae

objconvert -yz -gl -n glass -d -s cylinder1_001-mesh -o cylinder.inc teeth.dae

objconvert -uv -yz -gl -n table -d -s Plane-mesh -o table.inc teeth.dae
