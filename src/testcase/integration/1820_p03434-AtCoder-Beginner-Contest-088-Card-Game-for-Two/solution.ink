// Translated from solution.cpp.

func main() -> dynamic
{
  var N: dynamic = cpp_uninitialized();
  var k: dynamic = 0;
  read(N);
  {
    var i: dynamic = 0;
    while ((i < N))
    {
      read(A.at(i));
      i += 1;
    }
  }
  sort(A.begin(), A.end(), greater());
  {
    var i: dynamic = 0;
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
