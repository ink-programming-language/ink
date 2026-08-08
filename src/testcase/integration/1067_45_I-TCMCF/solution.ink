// Translated from solution.cpp.

func cmp(n1: dynamic, n2: dynamic)
{
  return (((*(cpp_cast(n1)))) - ((*(cpp_cast(n2)))));
}

func main()
{
  var ar = cpp_array(100);
  var n: dynamic;
  var neg = 0;
  var qna = 0;
  scanf("%d", (&n));
  {
    var i = 0;
    while ((i < n))
    {
      scanf("%d", (ar + i));
      i += 1;
    }
  }
  qsort(ar, n, cpp_sizeof(dynamic), cmp);
  if ((ar[0] > 0))
  {
    {
      var i = 0;
      while ((i < n))
      {
        printf("%d ", ar[i]);
        i += 1;
      }
    }
    return 0;
  }
  {
    var i = 0;
    while ((ar[i] < 0))
    {
      neg += 1;
      i += 1;
    }
  }
  if ((((neg % 2)) == 1))
  {
    neg -= 1;
  }
  {
    var i = 0;
    while ((i < neg))
    {
      printf("%d ", ar[i]);
      qna = 1;
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < n))
    {
      if ((ar[i] > 0))
      {
        printf("%d ", ar[i]);
        qna = 1;
      }
      i += 1;
    }
  }
  if ((!qna))
  {
    printf("%d", ar[(n - 1)]);
  }
  return 0;
}
