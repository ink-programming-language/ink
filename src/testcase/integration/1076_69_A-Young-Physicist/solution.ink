// Translated from solution.cpp.

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
  var z: dynamic = cpp_uninitialized();
  scanf("%d", (&n));
  var arr: dynamic = cpp_array(3, 1);
  arr[0][0] = cpp_assign(arr[0][1], "=", cpp_assign(arr[0][2], "=", 0));
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      scanf("%d %d %d", (&x), (&y), (&z));
      arr[0][0] += x;
      arr[0][1] += y;
      arr[0][2] += z;
      i += 1;
    }
  }
  if ((((arr[0][0] == 0) && (arr[0][1] == 0)) && (arr[0][2] == 0)))
  {
    printf("YES");
  } else
  {
    printf("NO");
  }
  return 0;
}
