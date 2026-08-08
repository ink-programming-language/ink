// Translated from solution.cpp.

var n: dynamic;

var myodd: dynamic;

var myeven: dynamic;

func cmp(a: dynamic, b: dynamic)
{
  return (a > b);
}

func main()
{
  scanf("%d", (&n));
  var tmp: dynamic;
  var sum = 0;
  {
    var i = 0;
    while ((i < n))
    {
      scanf("%d", (&tmp));
      sum += tmp;
      if (((tmp % 2) == 0))
      {
        myeven.push_back(tmp);
      } else
      {
        myodd.push_back(tmp);
      }
      i += 1;
    }
  }
  sort(myeven.begin(), myeven.end(), cmp);
  sort(myodd.begin(), myodd.end(), cmp);
  var ans1 = 0;
  var len1 = myeven.size();
  var len2 = myodd.size();
  var i = 0;
  var j = 0;
  {
    while (((i < len1) && (j < len2)))
    {
      ans1 += myeven[i];
      ans1 += myodd[j];
      i += 1;
      j += 1;
    }
  }
  if ((i < len1))
  {
    ans1 += myeven[i];
  }
  if ((j < len2))
  {
    ans1 += myodd[j];
  }
  var ans2 = 0;
  i = 0;
  j = 0;
  {
    while (((i < len1) && (j < len2)))
    {
      ans2 += myodd[j];
      ans2 += myeven[i];
      i += 1;
      j += 1;
    }
  }
  if ((j < len2))
  {
    ans2 += myodd[j];
  }
  if ((i < len1))
  {
    ans2 += myeven[i];
  }
  printf("%d\n", (sum - max(ans1, ans2)));
  return 0;
}
