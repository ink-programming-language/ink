// Translated from solution.cpp.

func main()
{
  var n: dynamic;
  var f: dynamic;
  var sum = 0;
  read(n);
  {
    var i = 0;
    while ((i < n))
    {
      read(f);
      sum += f;
      i += 1;
    }
  }
  var count = 0;
  var demo = sum;
  {
    var j = 1;
    while ((j <= 5))
    {
      demo += 1;
      if (((demo % ((n + 1))) != 1))
      {
        count += 1;
      }
      j += 1;
    }
  }
  write(count);
  return 0;
}
