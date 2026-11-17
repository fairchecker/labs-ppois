/**
 * @file test_collection.cpp
 * @brief Unit tests for exhibits, departments, storage rooms and showcases.
 */

#include <gtest/gtest.h>

#include <string>

#include "museum/artifact.hpp"
#include "museum/department.hpp"
#include "museum/exceptions.hpp"
#include "museum/fossil.hpp"
#include "museum/jewelry.hpp"
#include "museum/manuscript.hpp"
#include "museum/painting.hpp"
#include "museum/sculpture.hpp"
#include "museum/showcase.hpp"
#include "museum/storage_room.hpp"
#include "museum/textile.hpp"
#include "museum/weapon.hpp"

namespace museum {
namespace {

Artifact MakeArtifact() {
    return Artifact("a1", "Bronze bowl", "Ancient", 80, 1000.0, "bronze", 2.0, "Greece");
}

Painting MakePainting(double widthCm, double heightCm) {
    return Painting("p1", "Landscape", "Modern", 80, 5000.0, 3.0, "France", "Claude Monet",
                    "oil on canvas", widthCm, heightCm);
}

}  // namespace

TEST(ExhibitTest, RejectsInvalidRecord) {
    EXPECT_THROW(Artifact("", "Title", "Epoch", 50, 100.0, "wood", 1.0, "Place"),
                 InvalidExhibitException);
    EXPECT_THROW(Artifact("a", "", "Epoch", 50, 100.0, "wood", 1.0, "Place"),
                 InvalidExhibitException);
    EXPECT_THROW(Artifact("a", "Title", "Epoch", 50, -1.0, "wood", 1.0, "Place"),
                 InvalidExhibitException);
    EXPECT_THROW(Artifact("a", "Title", "Epoch", 101, 100.0, "wood", 1.0, "Place"),
                 InvalidConditionException);
}

TEST(ExhibitTest, AssessesConditionAndLoanEligibility) {
    Artifact artifact = MakeArtifact();
    EXPECT_TRUE(artifact.IsLoanable());
    artifact.AssessCondition(60);
    EXPECT_FALSE(artifact.IsLoanable());
    artifact.AssessCondition(70);
    EXPECT_TRUE(artifact.IsLoanable());
    EXPECT_THROW(artifact.AssessCondition(101), InvalidConditionException);
}

TEST(ExhibitTest, DepreciatesValue) {
    Artifact artifact = MakeArtifact();
    artifact.Depreciate(10.0);
    EXPECT_DOUBLE_EQ(artifact.AppraisedValue(), 900.0);
    artifact.Depreciate(100.0);
    EXPECT_DOUBLE_EQ(artifact.AppraisedValue(), 0.0);
    EXPECT_THROW(artifact.Depreciate(-1.0), InvalidExhibitException);
    EXPECT_THROW(artifact.Depreciate(101.0), InvalidExhibitException);
}

TEST(ExhibitTest, SummaryMentionsCategoryAndCondition) {
    Artifact artifact = MakeArtifact();
    EXPECT_EQ(artifact.Category(), "artifact");
    EXPECT_NE(artifact.Summary().find("artifact: Bronze bowl"), std::string::npos);
    EXPECT_NE(artifact.Summary().find("condition 80"), std::string::npos);
    EXPECT_EQ(artifact.Id(), "a1");
    EXPECT_EQ(artifact.Title(), "Bronze bowl");
    EXPECT_EQ(artifact.Epoch(), "Ancient");
}

TEST(ArtifactTest, ValidatesMaterialWeightAndOrigin) {
    EXPECT_THROW(Artifact("a", "T", "E", 50, 100.0, "", 1.0, "Place"), InvalidExhibitException);
    EXPECT_THROW(Artifact("a", "T", "E", 50, 100.0, "wood", -1.0, "Place"),
                 InvalidExhibitException);
    EXPECT_THROW(Artifact("a", "T", "E", 50, 100.0, "wood", 1.0, ""), InvalidExhibitException);
}

TEST(ArtifactTest, MovesBetweenRooms) {
    Artifact artifact = MakeArtifact();
    EXPECT_TRUE(artifact.LocationRoomId().empty());
    artifact.MoveTo("room-1");
    EXPECT_EQ(artifact.LocationRoomId(), "room-1");
    EXPECT_THROW(artifact.MoveTo(""), InvalidExhibitException);
}

TEST(ArtifactTest, DetectsFragileMaterials) {
    Artifact glass("a", "T", "E", 50, 100.0, "glass", 2.0, "P");
    Artifact ceramic("b", "T", "E", 50, 100.0, "ceramic", 2.0, "P");
    Artifact light("c", "T", "E", 50, 100.0, "wood", 0.5, "P");
    Artifact sturdy("d", "T", "E", 50, 100.0, "wood", 5.0, "P");
    EXPECT_TRUE(glass.IsFragile());
    EXPECT_TRUE(ceramic.IsFragile());
    EXPECT_TRUE(light.IsFragile());
    EXPECT_FALSE(sturdy.IsFragile());
    EXPECT_EQ(sturdy.Material(), "wood");
    EXPECT_DOUBLE_EQ(sturdy.WeightKg(), 5.0);
}

TEST(PaintingTest, ComputesAreaAndSize) {
    Painting small = MakePainting(100.0, 50.0);
    Painting large = MakePainting(200.0, 200.0);
    EXPECT_DOUBLE_EQ(small.Area(), 5000.0);
    EXPECT_FALSE(small.IsLarge());
    EXPECT_TRUE(large.IsLarge());
    EXPECT_EQ(small.Category(), "painting");
    EXPECT_EQ(small.Artist(), "Claude Monet");
    EXPECT_EQ(small.Technique(), "oil on canvas");
}

TEST(PaintingTest, RejectsInvalidMetadata) {
    EXPECT_THROW(Painting("p", "T", "E", 50, 100.0, 1.0, "P", "", "oil", 10.0, 10.0),
                 InvalidExhibitException);
    EXPECT_THROW(Painting("p", "T", "E", 50, 100.0, 1.0, "P", "Artist", "oil", 0.0, 10.0),
                 InvalidExhibitException);
    EXPECT_THROW(Painting("p", "T", "E", 50, 100.0, 1.0, "P", "Artist", "oil", 10.0, -1.0),
                 InvalidExhibitException);
}

TEST(SculptureTest, EstimatesVolumeAndOutdoorUse) {
    Sculpture sculpture("s1", "Discobolus", "Ancient", 90, 7000.0, 50.0, "Greece", "marble",
                        200.0, true, false);
    EXPECT_DOUBLE_EQ(sculpture.VolumeEstimate(), 0.5);
    EXPECT_FALSE(sculpture.IsOutdoorSuitable());
    EXPECT_TRUE(sculpture.HasPedestal());
    EXPECT_DOUBLE_EQ(sculpture.HeightCm(), 200.0);
    EXPECT_EQ(sculpture.Category(), "sculpture");
    EXPECT_THROW(Sculpture("s", "T", "E", 50, 100.0, 10.0, "P", "stone", 0.0, false, true),
                 InvalidExhibitException);
}

TEST(ManuscriptTest, DetectsRareItems) {
    Manuscript oldBook("m1", "Codex", "Medieval", 70, 9000.0, 2.0, "Italy", 100, "Latin", 1500);
    Manuscript longBook("m2", "Chronicle", "Medieval", 70, 9000.0, 2.0, "Italy", 600, "Latin",
                        1700);
    Manuscript commonBook("m3", "Notes", "Modern", 70, 100.0, 0.5, "Italy", 100, "Italian", 1900);
    EXPECT_TRUE(oldBook.IsRare());
    EXPECT_TRUE(longBook.IsRare());
    EXPECT_FALSE(commonBook.IsRare());
    EXPECT_TRUE(oldBook.ReadableBy("Latin"));
    EXPECT_FALSE(oldBook.ReadableBy("English"));
    EXPECT_EQ(commonBook.Category(), "manuscript");
}

TEST(ManuscriptTest, RejectsInvalidMetadata) {
    EXPECT_THROW(Manuscript("m", "T", "E", 50, 100.0, 1.0, "P", 0, "Latin", 1500),
                 InvalidExhibitException);
    EXPECT_THROW(Manuscript("m", "T", "E", 50, 100.0, 1.0, "P", 10, "", 1500),
                 InvalidExhibitException);
    EXPECT_THROW(Manuscript("m", "T", "E", 50, 100.0, 1.0, "P", 10, "Latin", -1),
                 InvalidExhibitException);
}

TEST(FossilTest, DescribesAge) {
    Fossil recent("f1", "Shell", "Holocene", 60, 500.0, 1.0, "Sea", "Mollusc", "Holocene", 10.0);
    Fossil ancient("f2", "Bone", "Jurassic", 60, 5000.0, 5.0, "Land", "Dinosaur", "Jurassic",
                   150000.0);
    EXPECT_FALSE(recent.IsPrehistoric());
    EXPECT_TRUE(ancient.IsPrehistoric());
    EXPECT_NE(ancient.EraDescription().find("Jurassic"), std::string::npos);
    EXPECT_EQ(ancient.Species(), "Dinosaur");
    EXPECT_EQ(ancient.AgeThousandYears(), 150000.0);
    EXPECT_EQ(ancient.Category(), "fossil");
}

TEST(FossilTest, RejectsInvalidMetadata) {
    EXPECT_THROW(Fossil("f", "T", "E", 50, 100.0, 1.0, "P", "", "Era", 1.0),
                 InvalidExhibitException);
    EXPECT_THROW(Fossil("f", "T", "E", 50, 100.0, 1.0, "P", "Species", "", 1.0),
                 InvalidExhibitException);
    EXPECT_THROW(Fossil("f", "T", "E", 50, 100.0, 1.0, "P", "Species", "Era", -1.0),
                 InvalidExhibitException);
}

TEST(JewelryTest, AppraisesGemstonePremium) {
    Jewelry modest("j1", "Ring", "Modern", 90, 1000.0, 0.1, "France", "diamond", 2.0, true);
    Jewelry precious("j2", "Tiara", "Modern", 90, 1000.0, 0.5, "France", "diamond", 50.0, false);
    EXPECT_DOUBLE_EQ(modest.Appraise(), 3000.0);
    EXPECT_FALSE(modest.IsHighValue());
    EXPECT_DOUBLE_EQ(precious.Appraise(), 51000.0);
    EXPECT_TRUE(precious.IsHighValue());
    EXPECT_TRUE(modest.IsAuthenticated());
    EXPECT_EQ(modest.Gemstone(), "diamond");
    EXPECT_EQ(modest.Category(), "jewelry");
}

TEST(JewelryTest, RejectsInvalidMetadata) {
    EXPECT_THROW(Jewelry("j", "T", "E", 50, 100.0, 1.0, "P", "", 1.0, true),
                 InvalidExhibitException);
    EXPECT_THROW(Jewelry("j", "T", "E", 50, 100.0, 1.0, "P", "gold", -1.0, true),
                 InvalidExhibitException);
}

TEST(WeaponTest, DetectsAntiques) {
    Weapon oldSabre("w1", "Sabre", "Napoleonic", 60, 4000.0, 1.2, "France", "sabre", 1900, true);
    Weapon modernRifle("w2", "Rifle", "Modern", 60, 4000.0, 4.0, "Germany", "rifle", 1950, false);
    EXPECT_TRUE(oldSabre.IsAntique(2020));
    EXPECT_FALSE(modernRifle.IsAntique(2020));
    EXPECT_FALSE(oldSabre.IsAntique(1850));
    EXPECT_TRUE(oldSabre.RequiresLicense());
    EXPECT_FALSE(modernRifle.RequiresLicense());
    EXPECT_EQ(oldSabre.WeaponType(), "sabre");
    EXPECT_EQ(oldSabre.Category(), "weapon");
}

TEST(WeaponTest, RejectsInvalidMetadata) {
    EXPECT_THROW(Weapon("w", "T", "E", 50, 100.0, 1.0, "P", "", 1900, true),
                 InvalidExhibitException);
    EXPECT_THROW(Weapon("w", "T", "E", 50, 100.0, 1.0, "P", "sabre", -1, true),
                 InvalidExhibitException);
}

TEST(TextileTest, ComputesAreaAndClimateNeeds) {
    Textile silk("t1", "Kimono", "Edo", 50, 3000.0, 1.0, "Japan", "silk", 200.0, 100.0);
    Textile cotton("t2", "Shirt", "Modern", 50, 100.0, 0.2, "India", "cotton", 50.0, 30.0);
    EXPECT_DOUBLE_EQ(silk.Area(), 20000.0);
    EXPECT_TRUE(silk.NeedsClimateControl());
    EXPECT_FALSE(cotton.NeedsClimateControl());
    EXPECT_EQ(silk.Fabric(), "silk");
    EXPECT_EQ(silk.Category(), "textile");
}

TEST(TextileTest, RejectsInvalidMetadata) {
    EXPECT_THROW(Textile("t", "T", "E", 50, 100.0, 1.0, "P", "", 10.0, 10.0),
                 InvalidExhibitException);
    EXPECT_THROW(Textile("t", "T", "E", 50, 100.0, 1.0, "P", "silk", 0.0, 10.0),
                 InvalidExhibitException);
    EXPECT_THROW(Textile("t", "T", "E", 50, 100.0, 1.0, "P", "silk", 10.0, 0.0),
                 InvalidExhibitException);
}

TEST(DepartmentTest, ManagesExhibitCapacity) {
    Department department("Archaeology", 1, "c1", 2);
    EXPECT_TRUE(department.AddExhibit("ex1"));
    EXPECT_FALSE(department.AddExhibit("ex1"));
    EXPECT_TRUE(department.AddExhibit("ex2"));
    EXPECT_TRUE(department.IsFull());
    EXPECT_FALSE(department.AddExhibit("ex3"));
    EXPECT_EQ(department.ExhibitCount(), 2U);
    EXPECT_TRUE(department.RemoveExhibit("ex1"));
    EXPECT_FALSE(department.RemoveExhibit("ex1"));
    EXPECT_THROW(department.RequireExhibit("ex1"), ExhibitNotFoundException);
    department.RequireExhibit("ex2");
    EXPECT_EQ(department.Name(), "Archaeology");
    EXPECT_EQ(department.Floor(), 1);
    EXPECT_EQ(department.CuratorId(), "c1");
}

TEST(DepartmentTest, RejectsInvalidRecords) {
    EXPECT_THROW(Department("", 1, "c1", 2), InvalidDepartmentException);
    EXPECT_THROW(Department("Dep", 1, "", 2), InvalidDepartmentException);
    EXPECT_THROW(Department("Dep", -1, "c1", 2), InvalidDepartmentException);
    EXPECT_THROW(Department("Dep", 1, "c1", 0), InvalidDepartmentException);
}

TEST(DepartmentTest, RejectsEmptyExhibitId) {
    Department department("Archaeology", 1, "c1", 2);
    EXPECT_THROW(department.AddExhibit(""), InvalidExhibitException);
}

TEST(StorageRoomTest, ManagesCapacity) {
    StorageRoom room("r1", 2, 20.0, 50.0);
    EXPECT_TRUE(room.Store("ex1"));
    EXPECT_FALSE(room.Store("ex1"));
    EXPECT_TRUE(room.Store("ex2"));
    EXPECT_TRUE(room.IsFull());
    EXPECT_FALSE(room.Store("ex3"));
    EXPECT_TRUE(room.Remove("ex1"));
    EXPECT_FALSE(room.Remove("ex1"));
    EXPECT_FALSE(room.IsFull());
    EXPECT_EQ(room.ExhibitCount(), 1U);
    EXPECT_EQ(room.Id(), "r1");
    EXPECT_DOUBLE_EQ(room.Temperature(), 20.0);
    EXPECT_DOUBLE_EQ(room.Humidity(), 50.0);
}

TEST(StorageRoomTest, ChecksClimateLimits) {
    StorageRoom ok("r1", 10, 16.0, 40.0);
    StorageRoom hot("r2", 10, 24.1, 50.0);
    StorageRoom dry("r3", 10, 20.0, 39.9);
    EXPECT_TRUE(ok.ClimateOk());
    EXPECT_FALSE(hot.ClimateOk());
    EXPECT_FALSE(dry.ClimateOk());
}

TEST(StorageRoomTest, RejectsInvalidRecords) {
    EXPECT_THROW(StorageRoom("", 10, 20.0, 50.0), InvalidExhibitException);
    EXPECT_THROW(StorageRoom("r", 0, 20.0, 50.0), InvalidExhibitException);
    StorageRoom room("r", 2, 20.0, 50.0);
    EXPECT_THROW(room.Store(""), InvalidExhibitException);
}

TEST(ShowcaseTest, ManagesExhibitsAndLighting) {
    Showcase showcase("sc1", "room1", 2);
    EXPECT_FALSE(showcase.IsLit());
    showcase.TurnOn();
    EXPECT_TRUE(showcase.IsLit());
    showcase.TurnOff();
    EXPECT_FALSE(showcase.IsLit());
    EXPECT_TRUE(showcase.Place("ex1"));
    EXPECT_FALSE(showcase.Place("ex1"));
    EXPECT_TRUE(showcase.Place("ex2"));
    EXPECT_TRUE(showcase.IsFull());
    EXPECT_FALSE(showcase.Place("ex3"));
    EXPECT_TRUE(showcase.Remove("ex1"));
    EXPECT_FALSE(showcase.Remove("ex1"));
    EXPECT_EQ(showcase.ExhibitCount(), 1U);
    EXPECT_EQ(showcase.RoomId(), "room1");
}

TEST(ShowcaseTest, RejectsInvalidRecords) {
    EXPECT_THROW(Showcase("", "room", 2), InvalidExhibitException);
    EXPECT_THROW(Showcase("sc", "", 2), InvalidExhibitException);
    EXPECT_THROW(Showcase("sc", "room", 0), InvalidExhibitException);
    Showcase showcase("sc", "room", 2);
    EXPECT_THROW(showcase.Place(""), InvalidExhibitException);
}

TEST(CollectionPolymorphismTest, ExhibitPointerDispatchesCategory) {
    const Painting painting = MakePainting(10.0, 10.0);
    const Exhibit& exhibit = painting;
    EXPECT_EQ(exhibit.Category(), "painting");
    EXPECT_EQ(exhibit.Id(), "p1");
}

}  // namespace museum
