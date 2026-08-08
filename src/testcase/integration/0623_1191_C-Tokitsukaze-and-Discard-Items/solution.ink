// Translated from solution.cpp.

func main()
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  var n: dynamic;
  var m: dynamic;
  var k: dynamic;
  read(n, m, k);
  var arr = cpp_array(m);
  {
    var i = 0;
    while ((i < m))
    {
      read(arr[i]);
      i += 1;
    }
  }
  var ans = 0;
  var sub = 0;
  var g = 1;
  var temp: dynamic;
  var now = (((arr[0] - 1)) / k);
  {
    var i = 1;
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
