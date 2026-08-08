// Translated from solution.cpp.

func main()
{
  var n: dynamic;
  read(n);
  var counter = n;
  while ((n > 1))
  {
    var flag = false;
    {
      var i = 2;
      while (((i * i) <= n))
      {
        if (((n % i) == 0))
        {
          n = (n / cpp_cast(i));
          counter += n;
          flag = true;
          break;
        }
        i += 1;
      }
    }
    if ((!flag))
    {
      n = 1;
      counter += 1;
    }
  }
  write(counter);
}
