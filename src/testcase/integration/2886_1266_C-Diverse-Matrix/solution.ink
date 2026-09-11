// Translated from solution.cpp.

var dx: dynamic = [1, 0, -1, 0, -1, -1, 1, 1];

var dy: dynamic = [0, 1, 0, -1, -1, 1, -1, 1];

func fast() -> dynamic
{
  ios_base.sync_with_stdio(0);
  cin.tie(null);
  cout.tie(null);
}

var mxN: dynamic = (3e5 + 5);

var oo: dynamic = 0x3f3f3f3f;

var mod: dynamic = (1e9 + 7);

func solve() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  read(n, m);
  if (((n == m) && (n == 1)))
  {
    write(0, "\n");
    return;
  }
  if ((n == 1))
  {
    {
      var i: dynamic = 1;
      while ((i < (m + 1)))
      {
        write((i + 1), cpp_char(" "));
        i += 1;
      }
    }
  } else if ((m == 1))
  {
    {
      var i: dynamic = 1;
      while ((i < (n + 1)))
      {
        write((i + 1), "\n");
        i += 1;
      }
    }
  } else
  {
    {
      var i: dynamic = 1;
      while ((i < (n + 1)))
      {
        {
          var j: dynamic = 1;
          while ((j < (m + 1)))
          {
            write((i * ((j + n))), cpp_char(" "));
            j += 1;
          }
        }
        write("\n");
        i += 1;
      }
    }
  }
}

func main() -> dynamic
{
  fast();
  solve();
}
