--    Xbase project source code
--
--    This file contains SQL code to test the XBase SQL implementation
--
--    Copyright (C) 2000 Dave Berton (db@trolltech.com)
--
--    This library is free software; you can redistribute it and/or
--    modify it under the terms of the GNU Lesser General Public
--    License as published by the Free Software Foundation; either
--    version 2.1 of the License, or (at your option) any later version.
--
--    This library is distributed in the hope that it will be useful,
--    but WITHOUT ANY WARRANTY; without even the implied warranty of
--    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
--    Lesser General Public License for more details.
--
--    You should have received a copy of the GNU Lesser General Public
--    License along with this library; if not, write to the Free Software
--    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
--
--    Contact:
--
--      Mail:
--
--        Technology Associates, Inc.
--        XBase Project
--        1455 Deming Way --11
--        Sparks, NV 89434
--        USA
--
--      Email:
--
--        xbase@techass.com
--
--      See our website at:
--
--        xdb.sourceforge.net

create table alltypes (
n1 numeric,
n2 numeric( 19, 3 ),
c1 char( 50 ),
c2 char( 10 ),
d1 date
);

create unique index alltypesindex_primary on alltypes ( n1 ); 
create index alltypesindex_1 on alltypes ( n1, n2 ); 
create index alltypesindex_2 on alltypes ( c1 ); 
create index alltypesindex_3 on alltypes ( d1 ); 

insert into alltypes values( 1, 12, 'non-latin1: ���� ��', 'more', '2001-01-11'); insert
into alltypes values( 2, 22, 'csdfsdf', 'more', '2002-01-21'); insert
into alltypes values( 3, 32, 'xsdfsdf', 'more', '2003-01-12'); insert
into alltypes values( 4, 42, 'csdfsdf', 'more', '2004-01-14'); insert
into alltypes values( 5, 52, 'csdfsdf', 'more', '2005-01-15'); insert
into alltypes values( 6, 62, 'xsdfsdf', 'more', '2006-01-16'); insert
into alltypes values( 7, 72, 'swdfsdf', 'more', '2007-02-17'); insert
into alltypes values( 8, 82, 'sdsddfsdf', 'more', '2000-01-01');
insert into alltypes values( 9, 92, 'sfdfsdf', 'more', '2000-01-01');
insert into alltypes values( 10, 32, 'dsddfsdf', 'more',
'2000-01-01'); insert into alltypes values( 21, 32, 'fsdfsdf', 'more',
'2000-01-01'); insert into alltypes values( 22, 32, 'dsdfsdf', 'more',
'2000-01-01'); insert into alltypes values( 23, 32, 'dsdfsdf', 'more',
'2000-01-01'); insert into alltypes values( 24, 32, 'gsdfsdf', 'more',
'2000-01-01'); insert into alltypes values( 25, 32, 'sdfsdf', 'more',
'2000-01-01'); insert into alltypes values( 26, 32, 'sfdfsdf', 'more',
'2000-01-01'); insert into alltypes values( 27, 32, 'sddfsdf', 'more',
'2000-01-01'); insert into alltypes values( 28, 32, 'sddfsdf', 'more',
'2000-01-01'); insert into alltypes values( 29, 32, 'sdfsdf', 'more',
'2000-01-01'); insert into alltypes values( 210, 32, 'scdfsdf',
'more', '2000-01-01'); insert into alltypes values( 31, 24, 'sdffsdf',
'more', '2000-01-01'); insert into alltypes values( 32, 24, 'sdffsdf',
'more', '2000-01-01'); insert into alltypes values( 33, 24, 'sdcfsdf',
'more', '2000-01-01'); insert into alltypes values( 34, 24, 'sddfsdf',
'more', '2000-01-01'); insert into alltypes values( 35, 24, 'sdfsdf',
'more', '2000-01-01'); insert into alltypes values( 36, 24, 'sdfcsdf',
'more', '2000-01-01'); insert into alltypes values( 37, 24, 'sdffsdf',
'more', '2000-01-01'); insert into alltypes values( 38, 24, 'sdffsdf',
'more', '2000-01-01'); insert into alltypes values( 39, 24, 'sdfvsdf',
'more', '2000-01-01'); insert into alltypes values( 310, 24,
'sbdfsdf', 'more', '2000-01-01'); insert into alltypes values( 41, 52,
'sdgfsdf', 'more', '2000-01-01'); insert into alltypes values( 42, 52,
'sddfsdf', 'more', '2000-01-01'); insert into alltypes values( 43, 52,
'sddfsdf', 'more', '2000-01-01'); insert into alltypes values( 44, 52,
'sddfsdf', 'more', '2000-01-01'); insert into alltypes values( 45, 52,
'sddfsdf', 'more', '2000-01-01'); insert into alltypes values( 46, 52,
'sdgfsdf', 'more', '2000-01-01'); insert into alltypes values( 47, 52,
'sdgfsdf', 'more', '2000-01-01'); insert into alltypes values( 48, 52,
'sdgfsdf', 'more', '2000-01-01'); insert into alltypes values( 49, 52,
'sdfgsdf', 'more', '2000-01-01'); insert into alltypes values( 410,
52, 'sdffsdf', 'more', '2000-01-01'); insert into alltypes values( 51,
62, 'sdffsdf', 'more', '2000-01-01'); insert into alltypes values( 52,
62, 'sdffsdf', 'more', '2000-01-01'); insert into alltypes values( 53,
62, 'sdffsdf', 'more', '2000-01-01'); insert into alltypes values( 54,
62, 'sdffsdf', 'more', '2000-01-01'); insert into alltypes values( 55,
62, 'sdffsdf', 'more', '2000-01-01'); insert into alltypes values( 56,
62, 'sdfsfdf', 'more', '2000-01-01'); insert into alltypes values( 57,
62, 'sdffsdf', 'more', '2000-01-01'); insert into alltypes values( 58,
62, 'sdffsdf', 'more', '2000-01-01'); insert into alltypes values( 59,
62, 'sfdfsdf', 'more', '2000-01-01'); insert into alltypes values(
510, 62, 'sdfdfgsdf', 'more', '2000-01-01'); insert into alltypes
values( 61, 72, 'sdfdfgsdf', 'more', '2000-01-01'); insert into
alltypes values( 62, 72, 'sdfdfgsdf', 'more', '2000-01-01'); insert
into alltypes values( 63, 72, 'sdfdfsdf', 'more', '2000-01-01');
insert into alltypes values( 64, 72, 'sdfsddf', 'more', '2000-01-01');
insert into alltypes values( 65, 72, 'sdrtfsdf', 'more',
'2000-01-01'); insert into alltypes values( 66, 72, 'sdrfsdf', 'more',
'2000-01-01'); insert into alltypes values( 67, 72, 'sdfssdfsdfdf',
'more', '2000-01-01'); insert into alltypes values( 68, 72,
'sdfsdsdfsdff', 'more', '2000-01-01'); insert into alltypes values(
69, 72, 'sdsdffsdf', 'more', '2000-01-01'); insert into alltypes
values( 610, 72, 'sdfssdfsdfdf', 'more', '2000-01-01');

insert into alltypes values( 91, 12, 'csdfsdf', 'more', '2000-01-01');
insert into alltypes values( 92, 22, 'csdfsdf', 'more', '2000-01-01');
insert into alltypes values( 93, 32, 'xsdfsdf', 'more', '2000-01-01');
insert into alltypes values( 94, 42, 'csdfsdf', 'more', '2000-01-01');
insert into alltypes values( 95, 52, 'csdfsdf', 'more', '2000-01-01');
insert into alltypes values( 96, 62, 'xsdfsdf', 'more', '2000-01-01');
insert into alltypes values( 97, 72, 'swdfsdf', 'more', '2000-01-01');
insert into alltypes values( 98, 82, 'sdsddfsdf', 'more', '2000-01-01');
insert into alltypes values( 99, 92, 'sfdfsdf', 'more', '2000-01-01');
insert into alltypes values( 910, 32, 'dsddfsdf', 'more', '2000-01-01');
insert into alltypes values( 921, 32, 'fsdfsdf', 'more', '2000-01-01');
insert into alltypes values( 922, 32, 'dsdfsdf', 'more', '2000-01-01');
insert into alltypes values( 923, 32, 'dsdfsdf', 'more', '2000-01-01');
insert into alltypes values( 924, 32, 'gsdfsdf', 'more', '2000-01-01');
insert into alltypes values( 925, 32, 'sdfsdf', 'more', '2000-01-01');
insert into alltypes values( 926, 32, 'sfdfsdf', 'more', '2000-01-01');
insert into alltypes values( 927, 32, 'sddfsdf', 'more', '2000-01-01');
insert into alltypes values( 928, 32, 'sddfsdf', 'more', '2000-01-01');
insert into alltypes values( 929, 32, 'sdfsdf', 'more', '2000-01-01');
insert into alltypes values( 9210, 32, 'scdfsdf', 'more', '2000-01-01');
insert into alltypes values( 931, 24, 'sdffsdf', 'more', '2000-01-01');
insert into alltypes values( 932, 24, 'sdffsdf', 'more', '2000-01-01');
insert into alltypes values( 933, 24, 'sdcfsdf', 'more', '2000-01-01');
insert into alltypes values( 934, 24, 'sddfsdf', 'more', '2000-01-01');
insert into alltypes values( 935, 24, 'sdfsdf', 'more', '2000-01-01');
insert into alltypes values( 936, 24, 'sdfcsdf', 'more', '2000-01-01');
insert into alltypes values( 937, 24, 'sdffsdf', 'more', '2000-01-01');
insert into alltypes values( 938, 24, 'sdffsdf', 'more', '2000-01-01');
insert into alltypes values( 939, 24, 'sdfvsdf', 'more', '2000-01-01');
insert into alltypes values( 9310, 24, 'sbdfsdf', 'more', '2000-01-01');
insert into alltypes values( 941, 52, 'sdgfsdf', 'more', '2000-01-01');
insert into alltypes values( 942, 52, 'sddfsdf', 'more', '2000-01-01');
insert into alltypes values( 943, 52, 'sddfsdf', 'more', '2000-01-01');
insert into alltypes values( 944, 52, 'sddfsdf', 'more', '2000-01-01');
insert into alltypes values( 945, 52, 'sddfsdf', 'more', '2000-01-01');
insert into alltypes values( 946, 52, 'sdgfsdf', 'more', '2000-01-01');
insert into alltypes values( 947, 52, 'sdgfsdf', 'more', '2000-01-01');
insert into alltypes values( 948, 52, 'sdgfsdf', 'more', '2000-01-01');
insert into alltypes values( 949, 52, 'sdfgsdf', 'more', '2000-01-01');
insert into alltypes values( 9410, 52, 'sdffsdf', 'more', '2000-01-01');
insert into alltypes values( 951, 62, 'sdffsdf', 'more', '2000-01-01');
insert into alltypes values( 952, 62, 'sdffsdf', 'more', '2000-01-01');
insert into alltypes values( 953, 62, 'sdffsdf', 'more', '2000-01-01');
insert into alltypes values( 954, 62, 'sdffsdf', 'more', '2000-01-01');
insert into alltypes values( 955, 62, 'sdffsdf', 'more', '2000-01-01');
insert into alltypes values( 956, 62, 'sdfsfdf', 'more', '2000-01-01');
insert into alltypes values( 957, 62, 'sdffsdf', 'more', '2000-01-01');
insert into alltypes values( 958, 62, 'sdffsdf', 'more', '2000-01-01');
insert into alltypes values( 959, 62, 'sfdfsdf', 'more', '2000-01-01');
insert into alltypes values( 9510, 62, 'sdfdfgsdf', 'more', '2000-01-01');
insert into alltypes values( 961, 72, 'sdfdfgsdf', 'more', '2000-01-01');
insert into alltypes values( 962, 72, 'sdfdfgsdf', 'more', '2000-01-01');
insert into alltypes values( 963, 72, 'sdfdfsdf', 'more', '2000-01-01');
insert into alltypes values( 964, 72, 'sdfsddf', 'more', '2000-01-01');
insert into alltypes values( 965, 72, 'sdrtfsdf', 'more', '2000-01-01');
insert into alltypes values( 966, 72, 'sdrfsdf', 'more', '2000-01-01');
insert into alltypes values( 967, 72, 'sdfssdfsdfdf', 'more', '2000-01-01');
insert into alltypes values( 968, 72, 'sdfsdsdfsdff', 'more', '2000-01-01');
insert into alltypes values( 969, 72, 'sdsdffsdf', 'more', '2000-01-01');
insert into alltypes values( 9699, 72, 'sdfssdfsdfdf', 'more', '2000-01-01');

