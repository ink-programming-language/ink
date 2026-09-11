// Translated from solution.cpp.

func main() -> dynamic
{
  ios_base.sync_with_stdio(false);
  var n: dynamic = cpp_uninitialized();
  read(n);
  var answer: dynamic = 0;
  {
    var i: dynamic = 1;
    while ((answer == 0))
    {
      var c: dynamic = 0;
      {
        var j: dynamic = 1;
        while (((j * j) <= i))
        {
          if (((i % j) == 0))
          {
            c += 1;
            if (((j * j) != i))
            {
              c += 1;
            }
          }
          j += 1;
        }
      }
      if ((c == n))
      {
        answer = i;
      }
      i += 1;
    }
  }
  write(answer, "\n");
  return 0;
}
