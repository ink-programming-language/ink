// Translated from solution.cpp.

func Get_Int()
{
  var Num = 0;
  var Flag = 1;
  var ch: dynamic;
  while (true)
  {
    ch = getchar();
    if ((ch == cpp_char("-")))
    {
      Flag = (-Flag);
    }
    if (!((((ch < cpp_char("0")) || (ch > cpp_char("9"))))))
    {
      break;
    }
  }
  while (true)
  {
    Num = (((Num * 10) + ch) - cpp_char("0"));
    ch = getchar();
    if (!((((ch >= cpp_char("0")) && (ch <= cpp_char("9"))))))
    {
      break;
    }
  }
  return (Num * Flag);
}

class Data
{
  var Left: dynamic;
  var Right: dynamic;
  var Sum: dynamic;
  func operator_less(a: dynamic)
  {
      return (Sum < a.Sum);
    }
  func operator_add(a: dynamic)
  {
      return [Left, a.Right, (Sum + a.Sum)];
    }
}

class Node
{
  var LeftMax: dynamic;
  var RightMax: dynamic;
  var Max: dynamic;
  var LeftMin: dynamic;
  var RightMin: dynamic;
  var Min: dynamic;
  var Sum: dynamic;
  func operator_add(a: dynamic)
  {
      var x: dynamic;
      x.LeftMax = max(LeftMax, (Sum + a.LeftMax));
      x.RightMax = max(a.RightMax, (RightMax + a.Sum));
      x.Max = max(x.LeftMax, x.RightMax);
      x.Max = max(x.Max, Max);
      x.Max = max(x.Max, a.Max);
      x.Max = max(x.Max, (RightMax + a.LeftMax));
      x.LeftMin = min(LeftMin, (Sum + a.LeftMin));
      x.RightMin = min(a.RightMin, (RightMin + a.Sum));
      x.Min = min(x.LeftMin, x.RightMin);
      x.Min = min(x.Min, Min);
      x.Min = min(x.Min, a.Min);
      x.Min = min(x.Min, (RightMin + a.LeftMin));
      x.Sum = (Sum + a.Sum);
      return x;
    }
}

var A = cpp_array((200005 * 4));

var Reverse = cpp_array((200005 * 4));

func Push_Down(Now: dynamic)
{
  if (Reverse[Now])
  {
    swap(A[Now].LeftMax, A[Now].LeftMin);
    swap(A[Now].RightMax, A[Now].RightMin);
    swap(A[Now].Min, A[Now].Max);
    A[Now].LeftMax.Sum *= -1;
    A[Now].RightMax.Sum *= -1;
    A[Now].Max.Sum *= -1;
    A[Now].LeftMin.Sum *= -1;
    A[Now].RightMin.Sum *= -1;
    A[Now].Min.Sum *= -1;
    A[Now].Sum.Sum *= -1;
    Reverse[(Now << 1)] ^= 1;
    Reverse[((Now << 1) | 1)] ^= 1;
    Reverse[Now] = false;
  }
}

func Update(Now: dynamic)
{
  Push_Down((Now << 1));
  Push_Down(((Now << 1) | 1));
  A[Now] = (A[(Now << 1)] + A[((Now << 1) | 1)]);
}

func Build(Now: dynamic, Value: dynamic, Left: dynamic, Right: dynamic)
{
  if ((Left == Right))
  {
    A[Now].LeftMin = cpp_assign(A[Now].RightMin, "=", cpp_assign(A[Now].Min, "=", cpp_assign(A[Now].LeftMax, "=", cpp_assign(A[Now].RightMax, "=", cpp_assign(A[Now].Max, "=", cpp_assign(A[Now].Sum, "=", [Left, Left, Value[Left]]))))));
    return;
  }
  var Mid = ((Left + Right) >> 1);
  Build((Now << 1), Value, (if (0) (Mid + 1) else Left), (if (0) Right else Mid));
  Build(((Now << 1) | 1), Value, (if (1) (Mid + 1) else Left), (if (1) Right else Mid));
  Update(Now);
}

func Modify(Now: dynamic, Position: dynamic, Value: dynamic, Left: dynamic, Right: dynamic)
{
  Push_Down(Now);
  if ((Left == Right))
  {
    A[Now].LeftMin = cpp_assign(A[Now].RightMin, "=", cpp_assign(A[Now].Min, "=", cpp_assign(A[Now].LeftMax, "=", cpp_assign(A[Now].RightMax, "=", cpp_assign(A[Now].Max, "=", cpp_assign(A[Now].Sum, "=", [Left, Left, Value]))))));
    return;
  }
  var Mid = ((Left + Right) >> 1);
  var i = (Position > Mid);
  Modify(((Now << 1) | i), Position, Value, (if (i) (Mid + 1) else Left), (if (i) Right else Mid));
  Update(Now);
}

func Flip(Now: dynamic, left: dynamic, right: dynamic, Left: dynamic, Right: dynamic)
{
  Push_Down(Now);
  if (((left == Left) && (right == Right)))
  {
    Reverse[Now] ^= 1;
    return;
  }
  var Mid = ((Left + Right) >> 1);
  if (((left > Mid) || (right <= Mid)))
  {
    var i = (left > Mid);
    Flip(((Now << 1) | i), left, right, (if (i) (Mid + 1) else Left), (if (i) Right else Mid));
  } else
  {
    Flip((Now << 1), left, Mid, (if (0) (Mid + 1) else Left), (if (0) Right else Mid));
    Flip(((Now << 1) | 1), (Mid + 1), right, (if (1) (Mid + 1) else Left), (if (1) Right else Mid));
  }
  Update(Now);
}

func Query(Now: dynamic, left: dynamic, right: dynamic, Left: dynamic, Right: dynamic)
{
  Push_Down(Now);
  if (((left == Left) && (right == Right)))
  {
    return A[Now];
  }
  var Mid = ((Left + Right) >> 1);
  if (((left > Mid) || (right <= Mid)))
  {
    var i = (left > Mid);
    return Query(((Now << 1) | i), left, right, (if (i) (Mid + 1) else Left), (if (i) Right else Mid));
  }
  return (Query((Now << 1), left, Mid, (if (0) (Mid + 1) else Left), (if (0) Right else Mid)) + Query(((Now << 1) | 1), (Mid + 1), right, (if (1) (Mid + 1) else Left), (if (1) Right else Mid)));
}

var N: dynamic;

var Q: dynamic;

var Top: dynamic;

var Value = cpp_array(200005);

var temp1 = cpp_array(200005);

var temp2 = cpp_array(200005);

func main()
{
  read(N);
  {
    var i = 1;
    while ((i <= N))
    {
      Value[i] = Get_Int();
      i += 1;
    }
  }
  Segment_Tree.Build(1, Value, 1, N);
  read(Q);
  while (cpp_update(Q, "--"))
  {
    if (Get_Int())
    {
      var Left = Get_Int();
      var Right = Get_Int();
      var K = Get_Int();
      var Ans = 0;
      while (cpp_update(K, "--"))
      {
        var Now = Segment_Tree.Query(1, Left, Right, 1, N);
        if ((Now.Max.Sum <= 0))
        {
          break;
        }
        Ans += Now.Max.Sum;
        temp1[cpp_update(Top, "++")] = Now.Max.Left;
        temp2[Top] = Now.Max.Right;
        Segment_Tree.Flip(1, Now.Max.Left, Now.Max.Right, 1, N);
      }
      while (Top)
      {
        Segment_Tree.Flip(1, temp1[Top], temp2[Top], 1, N);
        Top -= 1;
      }
      printf("%lld\n", max(Ans, 0));
    } else
    {
      var x = Get_Int();
      var y = Get_Int();
      Segment_Tree.Modify(1, x, y, 1, N);
    }
  }
  return 0;
}
