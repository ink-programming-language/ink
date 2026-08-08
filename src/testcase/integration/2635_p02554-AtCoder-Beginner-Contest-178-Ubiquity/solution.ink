// Translated from solution.cpp.

func main()
{
  var n: dynamic;
  read(n);
  var a = 1;
  var b = 1;
  var c = 1;
  var ans: dynamic;
  {
    var i = 0;
    while ((i < n))
    {
      a = (((a * 10)) % 1000000007);
      b = (((b * 9)) % 1000000007);
      c = (((c * 8)) % 1000000007);
      i += 1;
    }
  }
  ans = ((((a - (2 * b)) + c)) % 1000000007);
  ans = (((ans + 1000000007)) % 1000000007);
  write(ans);
  return 0;
}
