// Translated from solution.cpp.

var inf: dynamic = 0x3f3f3f3f;

var MaxN: dynamic = (2e5 + 10);

class node
{
  var val: dynamic = cpp_uninitialized();
  var id: dynamic = cpp_uninitialized();
}

var arr: dynamic = cpp_array(110);

func cmp(a: dynamic, b: dynamic) -> dynamic
{
  if ((a.val == b.val))
  {
    return (a.id < b.id);
  }
  return (a.val > b.val);
}

var ans: dynamic = cpp_array(110);

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var w: dynamic = cpp_uninitialized();
  scanf("%d%d", (&n), (&w));
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      scanf("%d", (&arr[i].val));
      arr[i].id = i;
      i += 1;
    }
  }
  sort(arr, (arr + n), cmp);
  var p: dynamic = w;
  var flag: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      if (((p - (((arr[i].val + 1)) / 2)) < 0))
      {
        flag = 1;
        break;
      }
      p = (p - (((arr[i].val + 1)) / 2));
      ans[arr[i].id] = (((arr[i].val + 1)) / 2);
      i += 1;
    }
  }
  if (flag)
  {
    printf("-1\n");
    return 0;
  }
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      if (((arr[i].val - ans[arr[i].id]) <= p))
      {
        p = (p - ((arr[i].val - ans[arr[i].id])));
        ans[arr[i].id] = arr[i].val;
      } else
      {
        ans[arr[i].id] += p;
        p = 0;
      }
      if ((p == 0))
      {
        break;
      }
      i += 1;
    }
  }
  if ((p == 0))
  {
    {
      var i: dynamic = 0;
      while ((i < n))
      {
        if ((i == 0))
        {
          printf("%d", ans[0]);
        } else
        {
          printf(" %d", ans[i]);
        }
        i += 1;
      }
    }
  } else
  {
    printf("-1");
  }
  printf("\n");
  return 0;
}
