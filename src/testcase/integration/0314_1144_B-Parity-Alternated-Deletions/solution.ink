// Translated from solution.cpp.

var n: dynamic = cpp_uninitialized();

var myodd: dynamic = cpp_uninitialized();

var myeven: dynamic = cpp_uninitialized();

func cmp(a: dynamic, b: dynamic) -> dynamic
{
  return (a > b);
}

func main() -> dynamic
{
  scanf("%d", (&n));
  var tmp: dynamic = cpp_uninitialized();
  var sum: dynamic = 0;
  {
    var i: dynamic = 0;
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
  var ans1: dynamic = 0;
  var len1: dynamic = myeven.size();
  var len2: dynamic = myodd.size();
  var i: dynamic = 0;
  var j: dynamic = 0;
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
  var ans2: dynamic = 0;
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
