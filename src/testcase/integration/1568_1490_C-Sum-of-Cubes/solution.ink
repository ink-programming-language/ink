// Translated from solution.cpp.

var N: dynamic = (1e4 + 4);

var frq: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  ios_base.sync_with_stdio(false);
  cin.tie(0);
  var test: dynamic = cpp_uninitialized();
  read(test);
  {
    var i: dynamic = 1;
    while ((i < N))
    {
      frq[(((1 * i) * i) * i)] = true;
      i += 1;
    }
  }
  while (cpp_update(test, "--"))
  {
    var n: dynamic = cpp_uninitialized();
    read(n);
    var mark: dynamic = false;
    {
      var i: dynamic = 1;
      while ((((i * i) * i) < n))
      {
        var dif: dynamic = (n - ((i * i) * i));
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
