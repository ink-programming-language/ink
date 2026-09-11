// Translated from solution.cpp.

func main() -> dynamic
{
  var a: dynamic = cpp_uninitialized();
  var b: dynamic = cpp_uninitialized();
  var c: dynamic = cpp_uninitialized();
  var d: dynamic = cpp_uninitialized();
  var e: dynamic = cpp_uninitialized();
  var f: dynamic = cpp_uninitialized();
  var g: dynamic = cpp_uninitialized();
  var h: dynamic = cpp_uninitialized();
  var i: dynamic = cpp_uninitialized();
  var j: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  var l: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  var n: dynamic = cpp_uninitialized();
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
  var z: dynamic = cpp_uninitialized();
  var t: dynamic = cpp_uninitialized();
  read(t);
  while (cpp_update(t, "--"))
  {
    read(a, b);
    var pp: dynamic = cpp_array((a + 1));
    var mp: dynamic = cpp_array((a + 1));
    {
      i = 1;
      while ((i <= a))
      {
        read(c, d);
        pp[i] = make_pair(c, d);
        var ct: dynamic = 0;
        if ((i > 1))
        {
          {
            j = 1;
            while ((j < i))
            {
              if (((abs((pp[i].first - pp[j].first)) + abs((pp[i].second - pp[j].second))) <= b))
              {
                mp[i][j] = 1;
                mp[j][i] = 1;
              }
              j += 1;
            }
          }
        }
        i += 1;
      }
    }
    f = 0;
    {
      i = 1;
      while ((i <= a))
      {
        if ((mp[i].size() == (a - 1)))
        {
          write("1", "\n");
          f = 1;
          break;
        }
        i += 1;
      }
    }
    if ((f == 0))
    {
      write("-1", "\n");
    }
  }
}
