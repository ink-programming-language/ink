// Translated from solution.cpp.

func main()
{
  ios_base.sync_with_stdio(false);
  var n: dynamic;
  read(n);
  var answer = 0;
  {
    var i = 1;
    while ((answer == 0))
    {
      var c = 0;
      {
        var j = 1;
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
