// Translated from solution.cpp.

func main() -> dynamic
{
  var N: dynamic = cpp_uninitialized();
  read(N);
  {
    var i: dynamic = 0;
    while ((i < N))
    {
      read(L[i]);
      i += 1;
    }
  }
  sort(L.begin(), L.end());
  var cnt: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < N))
    {
      {
        var j: dynamic = (i + 1);
        while ((j < N))
        {
          {
            var k: dynamic = (j + 1);
            while ((k < N))
            {
              if (((((L[k] < (L[i] + L[j])) && (L[i] != L[j])) && (L[i] != L[k])) && (L[j] != L[k])))
              {
                cnt += 1;
              }
              k += 1;
            }
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  write(cnt, "\n");
}
