// Translated from solution.cpp.

var prime = cpp_array((1000000 + 5));

var in_cpp: dynamic;

var isp = cpp_array((1000000 + 5));

var d4 = [-1, 0, 1, 0];

var y4 = [0, -1, 0, 1];

var dx = [1, -1, 0, 0, -1, 1, 1, -1];

var dy = [1, -1, 1, -1, 0, 0, -1, 1];

var dxh = [1, -1, 1, -1, 2, 2, -2, -2];

var dyh = [2, 2, -2, -2, 1, -1, 1, -1];

var mat = cpp_array(1005, 1005);

var val = cpp_array(1005, 1005);

func reset()
{
  {
    var i = 0;
    while ((i < 1005))
    {
      memset(mat[i], 0, cpp_sizeof((mat[i])));
      memset(val[i], 0, cpp_sizeof((val[i])));
      i += 1;
    }
  }
}

func main()
{
  var n: dynamic;
  var m: dynamic;
  while (((cin >> n) >> m))
  {
    var v: dynamic;
    {
      var i = 0;
      while ((i < n))
      {
        var sr: dynamic;
        read(sr);
        v.push_back(sr);
        i += 1;
      }
    }
    var ans = 1;
    {
      var i = 0;
      while ((i < m))
      {
        var s: dynamic;
        {
          var j = 0;
          while ((j < n))
          {
            s.insert(v[j][i]);
            j += 1;
          }
        }
        ans *= s.size();
        ans %= 1000000007;
        i += 1;
      }
    }
    write(ans, "\n");
  }
  return 0;
}
