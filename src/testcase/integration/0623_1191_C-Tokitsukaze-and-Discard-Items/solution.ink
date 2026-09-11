// Translated from solution.cpp.

func main() -> dynamic
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  read(n, m, k);
  var arr: dynamic = cpp_array(m);
  {
    var i: dynamic = 0;
    while ((i < m))
    {
      read(arr[i]);
      i += 1;
    }
  }
  var ans: dynamic = 0;
  var sub: dynamic = 0;
  var g: dynamic = 1;
  var temp: dynamic = cpp_uninitialized();
  var now: dynamic = (((arr[0] - 1)) / k);
  {
    var i: dynamic = 1;
    while ((i < m))
    {
      temp = ((((arr[i] - 1) - sub)) / k);
      if ((now == temp))
      {
        g += 1;
      } else
      {
        sub += g;
        g = 1;
        temp = ((((arr[i] - 1) - sub)) / k);
        now = temp;
        ans += 1;
      }
      i += 1;
    }
  }
  write((ans + 1));
}
