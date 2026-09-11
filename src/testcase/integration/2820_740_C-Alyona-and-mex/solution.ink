// Translated from solution.cpp.

class TaskC
{
  var v: dynamic = cpp_uninitialized();
  var ans: dynamic = cpp_uninitialized();
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  func rec(i: dynamic, mx: dynamic) -> dynamic
  {
      var tmx: dynamic = mx;
      var ed: dynamic = 0;
      var j: dynamic = v[i].first;
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
  func solve(cin: dynamic, cout: dynamic) -> dynamic
  {
      read(n, m);
      v.resize(m);
      ans.resize(n, false);
      var mex: dynamic = 1e18;
      {
        var i: dynamic = 0;
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
      var tmex: dynamic = mex;
      {
        var i: dynamic = 0;
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

func main() -> dynamic
{
  var solver: dynamic = cpp_uninitialized();
  var in_cpp: dynamic = cpp_uninitialized();
  var out: dynamic = cpp_uninitialized();
  solver.solve(in_cpp, out);
  return 0;
}
