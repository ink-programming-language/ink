// Translated from solution.cpp.

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  read(n);
  var counter: dynamic = n;
  while ((n > 1))
  {
    var flag: dynamic = false;
    {
      var i: dynamic = 2;
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
