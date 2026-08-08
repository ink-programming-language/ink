// Translated from solution.cpp.

var arr = cpp_array(505000);

var ans: dynamic;

func updata(l: dynamic, r: dynamic)
{
  var i = (l + 1);
  var j = (r - 1);
  while ((i <= j))
  {
    arr[i] = arr[l];
    arr[j] = arr[r];
    i += 1;
    j -= 1;
  }
  ans = max(ans, (((r - l)) / 2));
}

func main()
{
  var n: dynamic;
  scanf("%d", (&n));
  ans = 0;
  var st = 0;
  var ed = 0;
  {
    var i = 0;
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
    var i = 0;
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
