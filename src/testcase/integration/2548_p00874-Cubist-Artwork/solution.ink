// Translated from solution.cpp.

var W: dynamic;

var D: dynamic;

var hw = cpp_array(21);

var hd = cpp_array(21);

func main()
{
  var h: dynamic;
  while (cpp_comma(((cin >> W) >> D), W))
  {
    {
      var i = 0;
      while ((i < 21))
      {
        hw[i] = 0;
        hd[i] = 0;
        i += 1;
      }
    }
    {
      var i = 0;
      while ((i < W))
      {
        read(h);
        hw[h] += 1;
        i += 1;
      }
    }
    {
      var i = 0;
      while ((i < D))
      {
        read(h);
        hd[h] += 1;
        i += 1;
      }
    }
    var ans = 0;
    {
      var i = 0;
      while ((i <= 20))
      {
        ans += (max(hd[i], hw[i]) * i);
        i += 1;
      }
    }
    write(ans, "\n");
  }
}
