// Translated from solution.cpp.

var BUBEN: dynamic = 550;

var MOD: dynamic = (1e9 + 7);

var BASE: dynamic = 29;

var MOD1: dynamic = 998244353;

var BASE1: dynamic = 31;

func getchar_nolock() -> dynamic
{
  return getchar_unlocked();
}

func putchar_nolock(i: dynamic) -> dynamic
{
  return putchar_unlocked(i);
}

func main() -> dynamic
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  var n: dynamic = cpp_uninitialized();
  read(n);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(arr[i]);
      i += 1;
    }
  }
  if ((n == 1))
  {
    write("1 1\n", (-arr[0]), "\n1 1\n0\n1 1\n0");
  } else
  {
    write("1 ", (n - 1), cpp_char("\n"));
    {
      var i: dynamic = 0;
      while (((i + 1) < n))
      {
        write((((n - 1)) * ((arr[i] % n))), cpp_char(" "));
        arr[i] += (((n - 1)) * ((arr[i] % n)));
        i += 1;
      }
    }
    write(cpp_char("\n"), n, cpp_char(" "), n, cpp_char("\n"));
    write((n - ((arr[(n - 1)] % n))), cpp_char("\n"));
    arr[(n - 1)] += (n - ((arr[(n - 1)] % n)));
    write(1, cpp_char(" "), n, cpp_char("\n"));
    {
      var i: dynamic = 0;
      while ((i < n))
      {
        write((-arr[i]), cpp_char(" "));
        i += 1;
      }
    }
  }
}
