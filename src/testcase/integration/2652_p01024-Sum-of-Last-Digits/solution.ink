// Translated from solution.cpp.

func main()
{
  var n: dynamic;
  var m: dynamic;
  var k: dynamic;
  read(n, m, k);
  n %= 10;
  m %= 4;
  var a = n;
  var d = cpp_array(4);
  {
    var i = 0;
    while ((i < 4))
    {
      d[i] = a;
      a = ((a * n) % 10);
      i += 1;
    }
  }
  var ans = 0;
  {
    var i = 0;
    while ((i < 4))
    {
      ans += ((k / 4) * d[((m * i) % 4)]);
      if ((i < (k % 4)))
      {
        ans += d[((m * i) % 4)];
      }
      i += 1;
    }
  }
  write(ans, "\n");
  return 0;
}
