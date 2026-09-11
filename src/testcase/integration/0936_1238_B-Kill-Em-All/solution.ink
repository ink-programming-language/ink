// Translated from solution.cpp.

var maxn: dynamic = (3e5 + 10);

var N: dynamic = cpp_uninitialized();

var R: dynamic = cpp_uninitialized();

var arr: dynamic = cpp_array(maxn);

var T: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  ios.sync_with_stdio(false);
  cin.tie(0);
  read(T);
  while (cpp_update(T, "--"))
  {
    read(N, R);
    {
      var i: dynamic = 1;
      while ((i <= N))
      {
        read(arr[i]);
        i += 1;
      }
    }
    sort((arr + 1), ((arr + 1) + N));
    N = ((unique((arr + 1), ((arr + 1) + N)) - arr) - 1);
    var sum: dynamic = 0;
    var ans: dynamic = 0;
    {
      var i: dynamic = N;
      while ((i >= 1))
      {
        if (((arr[i] - sum) <= 0))
        {
          break;
        }
        ans += 1;
        sum += R;
        i -= 1;
      }
    }
    write(ans, "\n");
  }
  return 0;
}
