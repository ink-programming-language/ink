// Translated from solution.cpp.

func main()
{
  cin.tie(0)->sync_with_stdio(0);
  cin.exceptions(cin.failbit);
  var n: dynamic;
  var k: dynamic;
  read(n, k);
  var tot = 0;
  var sumPos = 0;
  {
    var cpp_name = 0;
    while ((cpp_name < (k)))
    {
      var pos: dynamic;
      var num: dynamic;
      read(pos, num);
      pos -= 1;
      sumPos += (pos * num);
      sumPos %= n;
      tot += num;
      cpp_name += 1;
    }
  }
  if ((tot > n))
  {
    write(-1, "\n");
  } else if ((tot < n))
  {
    write(1, "\n");
  } else
  {
    var expected = ((((n - 1)) * n) / 2);
    if (((expected % n) == sumPos))
    {
      write(1, "\n");
    } else
    {
      write(-1, "\n");
    }
  }
}
