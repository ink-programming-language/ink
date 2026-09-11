// Translated from solution.cpp.

var prime: dynamic = cpp_array((1000000 + 5));

var in_cpp: dynamic = cpp_uninitialized();

var isp: dynamic = cpp_array((1000000 + 5));

var d4: dynamic = [-1, 0, 1, 0];

var y4: dynamic = [0, -1, 0, 1];

var dx: dynamic = [1, -1, 0, 0, -1, 1, 1, -1];

var dy: dynamic = [1, -1, 1, -1, 0, 0, -1, 1];

var dxh: dynamic = [1, -1, 1, -1, 2, 2, -2, -2];

var dyh: dynamic = [2, 2, -2, -2, 1, -1, 1, -1];

var mat: dynamic = cpp_array(1005, 1005);

var val: dynamic = cpp_array(1005, 1005);

func reset() -> dynamic
{
  {
    var i: dynamic = 0;
    while ((i < 1005))
    {
      memset(mat[i], 0, cpp_sizeof((mat[i])));
      memset(val[i], 0, cpp_sizeof((val[i])));
      i += 1;
    }
  }
}

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  while (((cin >> n) >> m))
  {
    var v: dynamic = cpp_uninitialized();
    {
      var i: dynamic = 0;
      while ((i < n))
      {
        var sr: dynamic = cpp_uninitialized();
        read(sr);
        v.push_back(sr);
        i += 1;
      }
    }
    var ans: dynamic = 1;
    {
      var i: dynamic = 0;
      while ((i < m))
      {
        var s: dynamic = cpp_uninitialized();
        {
          var j: dynamic = 0;
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
