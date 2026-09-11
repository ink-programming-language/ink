// Translated from solution.cpp.

var arr: dynamic = cpp_array(505000);

var ans: dynamic = cpp_uninitialized();

func updata(l: dynamic, r: dynamic) -> dynamic
{
  var i: dynamic = (l + 1);
  var j: dynamic = (r - 1);
  while ((i <= j))
  {
    arr[i] = arr[l];
    arr[j] = arr[r];
    i += 1;
    j -= 1;
  }
  ans = max(ans, (((r - l)) / 2));
}

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  scanf("%d", (&n));
  ans = 0;
  var st: dynamic = 0;
  var ed: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      scanf("%d", (&arr[i]));
      if (i)
      {
        if ((arr[i] != arr[(i - 1)]))
        {
          ed += 1;
        } else
        {
          updata(st, ed);
          st = i;
          ed = i;
        }
      }
      i += 1;
    }
  }
  updata(st, ed);
  printf("%d\n", ans);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      if (i)
      {
        printf(" ");
      }
      printf("%d", arr[i]);
      i += 1;
    }
  }
  printf("\n");
  return 0;
}
