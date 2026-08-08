// Translated from solution.cpp.

class TaskC
{
  var v: dynamic;
  var ans: dynamic;
  var n: dynamic;
  var m: dynamic;
  func rec(i: dynamic, mx: dynamic)
  {
      var tmx = mx;
      var ed = 0;
      var j = v[i].first;
      while (((j <= v[i].second) && ans[j]))
      {
        j += 1;
        ed = max(ed, ans[j]);
      }
      j += 1;
      {
        j;
        while (((tmx != ed) && (j <= v[i].second)))
        {
          ans[j] = cpp_update(tmx, "--");
          j += 1;
        }
      }
      if ((i != (n - 1)))
      {
        rec((i + 1), mx);
      }
    }
  func solve(cin: dynamic, cout: dynamic)
  {
      read(n, m);
      v.resize(m);
      ans.resize(n, false);
      var mex = 1e18;
      {
        var i = 0;
        while ((i < m))
        {
          read(v[i].first, v[i].second);
          v[i].first -= 1;
          v[i].second -= 1;
          mex = min(mex, (v[i].second - v[i].first));
          i += 1;
        }
      }
      sort(v.begin(), v.end());
      write((mex + 1), "\n");
      var tmex = mex;
      {
        var i = 0;
        while ((i < n))
        {
          write(tmex, cpp_char(" "));
          tmex -= 1;
          if ((tmex < 0))
          {
            tmex = mex;
          }
          i += 1;
        }
      }
    }
}

func main()
{
  var solver: dynamic;
  var in_cpp: dynamic;
  var out: dynamic;
  solver.solve(in_cpp, out);
  return 0;
}
