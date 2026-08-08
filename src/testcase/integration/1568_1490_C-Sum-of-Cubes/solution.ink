// Translated from solution.cpp.

var N = (1e4 + 4);

var frq: dynamic;

func main()
{
  ios_base.sync_with_stdio(false);
  cin.tie(0);
  var test: dynamic;
  read(test);
  {
    var i = 1;
    while ((i < N))
    {
      frq[(((1 * i) * i) * i)] = true;
      i += 1;
    }
  }
  while (cpp_update(test, "--"))
  {
    var n: dynamic;
    read(n);
    var mark = false;
    {
      var i = 1;
      while ((((i * i) * i) < n))
      {
        var dif = (n - ((i * i) * i));
        if ((frq[dif] == true))
        {
          mark = true;
          break;
        }
        i += 1;
      }
    }
    if (mark)
    {
      write("YES\n");
    } else
    {
      write("NO\n");
    }
  }
}
