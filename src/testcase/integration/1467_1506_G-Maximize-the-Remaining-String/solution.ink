// Translated from solution.cpp.

var T: dynamic = cpp_uninitialized();

var last: dynamic = cpp_array(205);

var vis: dynamic = cpp_array(205);

var cnt: dynamic = cpp_uninitialized();

var ans: dynamic = cpp_array(205);

var s: dynamic = cpp_uninitialized();

var st: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  read(T);
  while (cpp_update(T, "--"))
  {
    cnt = 0;
    memset(last, 0, cpp_sizeof((last)));
    memset(vis, 0, cpp_sizeof((vis)));
    read(s);
    {
      var i: dynamic = 0;
      while ((i < s.size()))
      {
        last[s[i]] = i;
        i += 1;
      }
    }
    {
      var i: dynamic = 0;
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
      var i: dynamic = cnt;
      while ((i >= 1))
      {
        write(ans[i]);
        i -= 1;
      }
    }
    write("\n");
  }
}
