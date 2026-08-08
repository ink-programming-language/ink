// Translated from solution.cpp.

func main()
{
  var n: dynamic;
  var x: dynamic;
  var y: dynamic;
  var z: dynamic;
  scanf("%d", (&n));
  var arr = cpp_array(3, 1);
  arr[0][0] = cpp_assign(arr[0][1], "=", cpp_assign(arr[0][2], "=", 0));
  {
    var i = 0;
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
