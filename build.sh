bzr whoami "Amaury Carvalho <amauryspires@gmail.com>"
bzr launchpad-login amaurycarvalho
#bzr init
bzr add *
today=$(date +"%x %r %Z")
bzr commit -m "Commit on $today by $USER"
bzr push lp:~amaurycarvalho/msxbas2rom/trunk
