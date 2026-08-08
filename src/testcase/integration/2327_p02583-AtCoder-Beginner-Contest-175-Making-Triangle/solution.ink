// Translated from solution.cpp.

func main()
{
  var N: dynamic;
  read(N);
  {
    var i = 0;
    while ((i < N))
    {
      read(L[i]);
      i += 1;
    }
  }
  sort(L.begin(), L.end());
  var cnt = 0;
  {
    var i = 0;
    while ((i < N))
    {
      {
        var j = (i + 1);
        while ((j < N))
        {
          {
            var k = (j + 1);
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
