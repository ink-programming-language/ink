// Translated from solution.cpp.

func main()
{
  var a: dynamic;
  var b: dynamic;
  var c: dynamic;
  var d: dynamic;
  var e: dynamic;
  var f: dynamic;
  var g: dynamic;
  var h: dynamic;
  var i: dynamic;
  var j: dynamic;
  var k: dynamic;
  var l: dynamic;
  var m: dynamic;
  var n: dynamic;
  var x: dynamic;
  var y: dynamic;
  var z: dynamic;
  var t: dynamic;
  read(t);
  while (cpp_update(t, "--"))
  {
    read(a, b);
    var pp = cpp_array((a + 1));
    var mp = cpp_array((a + 1));
    {
      i = 1;
      while ((i <= a))
      {
        read(c, d);
        pp[i] = make_pair(c, d);
        var ct = 0;
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
