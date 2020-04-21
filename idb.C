f 0444 root sys Versions/004-0134-165 dist/Versions/004-0134-165 3000 root%
f 0444 root sys Versions/004-0134-166 dist/Versions/004-0134-166 3000 root%
f 0444 root sys Versions/004-0134-167 dist/Versions/004-0134-167 3000 root%
f 0444 root sys Versions/004-0134-175 dist/Versions/004-0134-175 3000 usr%
f 0444 root sys Versions/004-0144-046 dist/Versions/004-0144-046 3000 upd%
f 0444 root sys Versions/004-0144-047 dist/Versions/004-0144-047 3000 upd%
f 0444 root sys Versions/004-0144-048 dist/Versions/004-0144-048 3000 upd%
f 0444 root sys Versions/004-0174-036 dist/Versions/004-0174-036 3000 demos%
f 0444 root sys Versions/004-0384-033 dist/Versions/004-0384-033 3000 nfs%
f 0444 root sys Versions/004-0384-034 dist/Versions/004-0384-034 3000 nfs%
f 0444 root sys Versions/004-0384-035 dist/Versions/004-0384-035 3000 nfs%
f 0444 root sys Versions/004-0394-033 dist/Versions/004-0394-033 3000 xns%
f 0444 root sys Versions/004-0394-034 dist/Versions/004-0394-034 3000 xns%
f 0444 root sys Versions/004-0394-035 dist/Versions/004-0394-035 3000 xns%
f 0444 root sys Versions/004-0631-027 dist/Versions/004-0631-027 3000 sys%
f 0444 root sys Versions/004-0631-034 dist/Versions/004-0631-034 3000 sysnfs%
f 0755 bin bin bin/cp bin/mv boot% links(bin/ln bin/mv) root% squish%
f 0755 bin bin bin/ln bin/mv after squish% upd%
f 0755 bin bin bin/ln bin/mv boot% links(bin/cp bin/mv) root%
f 0755 bin bin bin/mv bin/mv boot% links(bin/cp bin/ln) root% squish%
f 0755 root sys defaultboot sys/kernels/3000.tcp 3000 links(kernels/3000.tcp vmunix) nostrip root% squish%
f 0755 root sys etc/mkboot etc/mkboot.sh root% upd%
f 0755 root sys kernels/3000.nfs sys/kernels/3000.nfs 3000 nostrip shared nfs%
f 0755 root sys kernels/3000.tcp sys/kernels/3000.tcp 3000 links(defaultboot vmunix) nostrip root% squish%
f 0755 root sys kernels/3000.tcp sys/kernels/3000.tcp 3000 nostrip shared upd%
f 0755 root sys kernels/3000.xns sys/kernels/3000.xns 3000 nostrip shared xns%
f 0755 root sys stand/ipfex stand/cmd/ipfex/ipfex root% squish% upd%
f 0755 root sys stand/mdfex stand/cmd/mdfex/mdfex root% squish% upd%
f 0755 root sys stand/sifex stand/cmd/sifex/sifex 3000 root% upd%
f 0755 bin bin usr/bin/dbx usr.bin/dbx/dbx upd% usr%
f 4755 root sys usr/bin/edge usr.bin/edge/ip2/edge upd% usr%
f 0755 root sys usr/etc/hyp usr.etc/hyperchannel/hyp hyper upd% usr%
f 0755 root sys usr/etc/hyroute usr.etc/hyperchannel/hyroute/hyroute  hyper% upd% usr%
f 4755 root sys usr/lib/gsh usr.bin/edge/gsh/gsh upd% usr%
f 0644 bin bin usr/lib/libkgl3.a gl2/gl2/ip2/libkgl.a 3000 sys% sysnfs%
f 0644 bin bin usr/lib/libkgl3_p.a gl2/gl2/ip2/libkgl_p.a 3000 sys% sysnfs%
f 0555 demos games usr/people/demos/dog demos.gl2/src/flight/dog demos% usr%
f 0555 demos games usr/people/demos/flight demos.gl2/src/flight/flight demos% usr%
f 0444 root sys usr/sys/h/buf.h sys/h/buf.h sys% sysnfs%
f 0644 root sys usr/sys/kernels/3000nfs.a sys/kernels/3000nfs.a 3000 nostrip sysnfs%
f 0644 root sys usr/sys/kernels/3000tcp.a sys/kernels/3000tcp.a 3000 nostrip sys% sysnfs%
f 0644 root sys usr/sys/kernels/3000xns.a sys/kernels/3000xns.a 3000 nostrip sys% sysnfs%
f 0444 root sys usr/sys/multibusif/if_hyreg.h sys/multibusif/if_hyreg.h upd hyper sys sysnfs
f 0755 root sys vmunix sys/kernels/3000.tcp 3000 links(defaultboot kernels/3000.tcp) nostrip root% squish%
