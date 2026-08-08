// Translated from solution.cpp.

var INF = (1e9 + 9);

var LINF = (1e17 + 9);

var MD = 998244353;

func po(a: dynamic, b: dynamic)
{
  var ans = 1;
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

var N = (2e3 + 33);

var n: dynamic;

var m: dynamic;

var k: dynamic;

var t: dynamic;

var arr = cpp_array(N, N);

var s: dynamic;

var r = cpp_array(N);

var c = cpp_array(N);

var rr = cpp_array(N);

var cc = cpp_array(N);

func main()
{
  ios_base.sync_with_stdio(false);
  cout.tie(null);
  cin.tie(null);
  write(fixed);
  write(setprecision(7));
  read(n, m);
  {
    var i = 0;
    while ((i < n))
    {
      {
        var j = 0;
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
    var i = 0;
    while ((i < n))
    {
      sort(rr[i].begin(), rr[i].end());
      var l = -1;
      for (var x in rr[i])
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
    var i = 0;
    while ((i < m))
    {
      sort(cc[i].begin(), cc[i].end());
      var l = -1;
      for (var x in cc[i])
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
    var i = 0;
    while ((i < n))
    {
      {
        var j = 0;
        while ((j < m))
        {
          var R = (lower_bound(r[i].begin(), r[i].end(), arr[i][j]) - r[i].begin());
          var C = (lower_bound(c[j].begin(), c[j].end(), arr[i][j]) - c[j].begin());
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
