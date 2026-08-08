// Translated from solution.cpp.

func main()
{
  var N: dynamic;
  var k = 0;
  read(N);
  {
    var i = 0;
    while ((i < N))
    {
      read(A.at(i));
      i += 1;
    }
  }
  sort(A.begin(), A.end(), greater());
  {
    var i = 0;
    while ((i < N))
    {
      if (((i % 2) == 0))
      {
        k += A.at(i);
      } else
      {
        k -= A.at(i);
      }
      i += 1;
    }
  }
  write(k, "\n");
}
