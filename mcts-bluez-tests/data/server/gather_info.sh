find . | xargs file | grep -v directory | awk '{print $1}' | sed 's/://g' | xargs md5sum | awk '{print $2" , "$1}'
