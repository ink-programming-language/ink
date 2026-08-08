// Translated from solution.cpp.

func bits(x: dynamic)
{
  return if ((x == 0)) 0 else (1 + bits((x & ((x - 1)))));
}

var PI = acos(-1.0);

var eps = 1e-9;

var INF = 1000000000;

func nextString()
{
  var buf = cpp_array(1000000);
  scanf("%s", buf);
  return buf;
}

var dx = [0, 1, 0, -1];

var dy = [1, 0, -1, 0];

func gcd(a: dynamic, b: dynamic)
{
  return if ((b == 0)) a else gcd(b, (a % b));
}

func get_sort(n: dynamic)
{
  {
    int_cpp(i) = 0;
    while (((i) < (n)))
    {
      scanf("%d", (&in_cpp[i]));
      (i) += 1;
    }
  }
  var ans: dynamic;
  {
    int_cpp(i) = 0;
    while (((i) < (n)))
    {
      {
        int_cpp(j) = (0);
        while (((j) < ((n - 1))))
        {
          if ((in_cpp[(j + 1)] > in_cpp[j]))
          {
            swap(in_cpp[(j + 1)], in_cpp[j]);
            ans.push_back(make_pair(j, (j + 1)));
          }
          (j) += 1;
        }
      }
      (i) += 1;
    }
  }
  return ans;
}

func main()
{
  var n: dynamic;
  scanf("%d\n", (&n));
  var res1 = get_sort(n);
  var res2 = get_sort(n);
  reverse((res1).begin(), (res1).end());
  printf("%d\n", (cpp_cast((res1).size()) + cpp_cast((res2).size())));
  {
    int_cpp(i) = 0;
    while (((i) < (cpp_cast((res2).size()))))
    {
      printf("%d %d\n", (res2[i].first + 1), (res2[i].second + 1));
      (i) += 1;
    }
  }
  {
    int_cpp(i) = 0;
    while (((i) < (cpp_cast((res1).size()))))
    {
      printf("%d %d\n", (res1[i].first + 1), (res1[i].second + 1));
      (i) += 1;
    }
  }
  return 0;
}
