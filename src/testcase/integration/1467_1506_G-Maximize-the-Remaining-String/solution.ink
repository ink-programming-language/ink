// Translated from solution.cpp.

var T: dynamic;

var last = cpp_array(205);

var vis = cpp_array(205);

var cnt: dynamic;

var ans = cpp_array(205);

var s: dynamic;

var st: dynamic;

func main()
{
  read(T);
  while (cpp_update(T, "--"))
  {
    cnt = 0;
    memset(last, 0, cpp_sizeof((last)));
    memset(vis, 0, cpp_sizeof((vis)));
    read(s);
    {
      var i = 0;
      while ((i < s.size()))
      {
        last[s[i]] = i;
        i += 1;
      }
    }
    {
      var i = 0;
      while ((i < s.size()))
      {
        if (vis[s[i]])
        {
          i += 1;
          continue;
        }
        while ((((!st.empty()) && (st.top() < s[i])) && (last[st.top()] > i)))
        {
          vis[st.top()] = 0;
          st.pop();
        }
        st.push(s[i]);
        vis[s[i]] = 1;
        i += 1;
      }
    }
    while ((!st.empty()))
    {
      ans[cpp_update(cnt, "++")] = st.top();
      st.pop();
    }
    {
      var i = cnt;
      while ((i >= 1))
      {
        write(ans[i]);
        i -= 1;
      }
    }
    write("\n");
  }
}
