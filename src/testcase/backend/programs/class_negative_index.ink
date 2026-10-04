class Box
{
  field Value: i32;
};

func index(): i8
{
  return -1;
}

func main(): i32
{
  var Objects = [Box(42); 300];
  return Objects[index()].Value;
}
