// Translated from solution.cpp.

var INF: dynamic = (1e9 + 9);

var LINF: dynamic = (1e17 + 9);

var MD: dynamic = 998244353;

func po(a: dynamic, b: dynamic) -> dynamic
{
  var ans: dynamic = 1;
  while ((b > 0))
  {
    if ((b & 1))
    {
      ans = (((ans * a)) % MD);
    }
    a = (((a * a)) % MD);
    b /= 2;
  }
  return ans;
}

var N: dynamic = (2e3 + 33);

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var k: dynamic = cpp_uninitialized();

var t: dynamic = cpp_uninitialized();

var arr: dynamic = cpp_array(N, N);

var s: dynamic = cpp_uninitialized();

var r: dynamic = cpp_array(N);

var c: dynamic = cpp_array(N);

var rr: dynamic = cpp_array(N);

var cc: dynamic = cpp_array(N);

func main() -> dynamic
{
  ios_base.sync_with_stdio(false);
  cout.tie(null);
  cin.tie(null);
  write(fixed);
  write(setprecision(7));
  read(n, m);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      {
        var j: dynamic = 0;
        while ((j < m))
        {
          read(arr[i][j]);
          rr[i].push_back(arr[i][j]);
          cc[j].push_back(arr[i][j]);
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      sort(rr[i].begin(), rr[i].end());
      var l: dynamic = -1;
      for (var x: dynamic in rr[i])
      {
        if ((l != x))
        {
          r[i].push_back(x);
          l = x;
        }
      }
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < m))
    {
      sort(cc[i].begin(), cc[i].end());
      var l: dynamic = -1;
      for (var x: dynamic in cc[i])
      {
        if ((l != x))
        {
          c[i].push_back(x);
          l = x;
        }
      }
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      {
        var j: dynamic = 0;
        while ((j < m))
        {
          var R: dynamic = (lower_bound(r[i].begin(), r[i].end(), arr[i][j]) - r[i].begin());
          var C: dynamic = (lower_bound(c[j].begin(), c[j].end(), arr[i][j]) - c[j].begin());
          write((max(R, C) + max((r[i].size() - R), (c[j].size() - C))), " ");
          j += 1;
        }
      }
      write("\n");
      i += 1;
    }
  }
  return 0;
}
